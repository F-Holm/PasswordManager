#include "core/core.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "types/account_view.h"

namespace {

// Domain-separates the two independently-encrypted fields of an account so
// their ciphertexts (and authentication tags) can never be swapped for one
// another. Each label is followed by the account's uid in the AAD (see
// AccountAad), which also stops fields being moved between accounts.
constexpr std::string_view kDescriptionAad = "account.description";
constexpr std::string_view kDataAad = "account.data";
constexpr std::string_view kVerifierAad = "vault.verifier";

std::span<const std::byte> AsBytes(std::string_view value) noexcept {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
}

CoreStatus FromDBStatus(DBStatus status) noexcept {
  switch (status) {
    case DBStatus::kOk:
      return CoreStatus::kOk;
    case DBStatus::kNotFound:
      return CoreStatus::kNotFound;
    case DBStatus::kBusy:
      return CoreStatus::kBusy;
    case DBStatus::kAlreadyExists:
      return CoreStatus::kAlreadyExists;
    case DBStatus::kClosed:
      return CoreStatus::kLocked;
    case DBStatus::kError:
      break;
  }
  return CoreStatus::kError;
}

using Uid = std::array<std::byte, AccountSize::kUid>;

// Additional authenticated data for one field of the account `uid`:
// `label || uid`.
std::vector<std::byte> AccountAad(std::string_view label, const Uid& uid) {
  std::vector<std::byte> aad;
  aad.reserve(label.size() + uid.size());
  const std::span<const std::byte> label_bytes = AsBytes(label);
  aad.insert(aad.end(), label_bytes.begin(), label_bytes.end());
  aad.insert(aad.end(), uid.begin(), uid.end());
  return aad;
}

}  // namespace

std::string_view ToString(CoreStatus status) noexcept {
  switch (status) {
    case CoreStatus::kOk:
      return "ok";
    case CoreStatus::kNotFound:
      return "not found";
    case CoreStatus::kConflict:
      return "conflict";
    case CoreStatus::kBusy:
      return "busy";
    case CoreStatus::kLocked:
      return "locked";
    case CoreStatus::kWrongPassword:
      return "wrong password";
    case CoreStatus::kInvalidArgument:
      return "invalid argument";
    case CoreStatus::kAlreadyExists:
      return "already exists";
    case CoreStatus::kCorrupted:
      return "corrupted";
    case CoreStatus::kError:
      break;
  }
  return "error";
}

void AccountPlain::SetZero() noexcept {
  Crypto::SecureClear(std::as_writable_bytes(std::span(description)));
  Crypto::SecureClear(std::as_writable_bytes(std::span(email)));
  Crypto::SecureClear(std::as_writable_bytes(std::span(username)));
  Crypto::SecureClear(std::as_writable_bytes(std::span(password)));
  Crypto::SecureClear(std::as_writable_bytes(std::span(extra)));
}

AccountPlain::~AccountPlain() noexcept {
  SetZero();
}

void AccountEntry::SetZero() noexcept {
  id = -1;
  position = -1;
  Crypto::SecureClear(std::as_writable_bytes(std::span(description)));
}

AccountEntry::~AccountEntry() noexcept {
  SetZero();
}

bool SetAccountField(std::span<char> field, std::string_view value) noexcept {
  if (value.size() > field.size()) {
    return false;
  }
  std::memset(field.data(), 0, field.size());
  std::memcpy(field.data(), value.data(), value.size());
  return true;
}

std::string_view GetAccountField(std::span<const char> field) noexcept {
  const auto* end =
      static_cast<const char*>(std::memchr(field.data(), '\0', field.size()));
  const std::size_t length = end != nullptr
                                 ? static_cast<std::size_t>(end - field.data())
                                 : field.size();
  return {field.data(), length};
}

Core::Core() noexcept = default;

Core::~Core() {
  Close();
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): named for callers.
CoreStatus Core::Create(std::string_view db_path,
                        std::string_view master_password,
                        const Crypto::KdfParams& kdf_params) {
  Close();
  if (master_password.empty()) {
    return CoreStatus::kInvalidArgument;
  }

  VaultMeta meta;
  meta.kdf_params = kdf_params;
  if (!Crypto::RandomBytes(meta.kdf_salt)) {
    return CoreStatus::kError;
  }

  Key key{};
  if (!DeriveKey(master_password, meta, key)) {
    return CoreStatus::kInvalidArgument;
  }
  if (!SealVerifier(key, meta)) {
    Crypto::SecureClear(key);
    return CoreStatus::kError;
  }

  if (!db_.Open(db_path)) {
    Crypto::SecureClear(key);
    return CoreStatus::kError;
  }

  // Refuse to adopt a file that already holds accounts: they were encrypted
  // with some other key.
  const auto count = db_.Count();
  CoreStatus status = CoreStatus::kOk;
  if (!count.has_value()) {
    status = FromDBStatus(count.error());
  } else if (*count > 0) {
    status = CoreStatus::kAlreadyExists;
  } else {
    status = FromDBStatus(db_.CreateMeta(meta));
  }

  if (status != CoreStatus::kOk) {
    Crypto::SecureClear(key);
    db_.Close();
    return status;
  }

  master_key_ = key;
  Crypto::SecureClear(key);
  has_key_ = true;
  key_epoch_ = meta.key_epoch;
  return CoreStatus::kOk;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): named for callers.
CoreStatus Core::Unlock(std::string_view db_path,
                        std::string_view master_password) {
  Close();

  if (!db_.Open(db_path, DB::OpenMode::kExistingOnly)) {
    return std::filesystem::exists(std::filesystem::path(db_path))
               ? CoreStatus::kError
               : CoreStatus::kNotFound;
  }

  const auto meta = db_.GetMeta();
  if (!meta.has_value()) {
    db_.Close();
    return meta.error() == DBStatus::kError ? CoreStatus::kCorrupted
                                            : FromDBStatus(meta.error());
  }
  if (meta->version != VaultMeta::kCurrentVersion) {
    db_.Close();
    return CoreStatus::kCorrupted;
  }

  Key key{};
  if (!DeriveKey(master_password, *meta, key)) {
    db_.Close();
    return CoreStatus::kCorrupted;
  }
  if (!CheckVerifier(key, *meta)) {
    Crypto::SecureClear(key);
    db_.Close();
    return CoreStatus::kWrongPassword;
  }

  master_key_ = key;
  Crypto::SecureClear(key);
  has_key_ = true;
  key_epoch_ = meta->key_epoch;
  return CoreStatus::kOk;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): named for callers.
CoreStatus Core::ChangeMasterPassword(
    std::string_view current_password, std::string_view new_password,
    const std::optional<Crypto::KdfParams>& kdf_params) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }
  if (new_password.empty()) {
    return CoreStatus::kInvalidArgument;
  }

  // The body runs inside one write transaction, so no other program can add
  // or edit an account (under the old key) between reading every account
  // and committing the re-encrypted ones. `result` carries the precise
  // reason when the body aborts.
  CoreStatus result = CoreStatus::kOk;
  Key old_key{};
  Key new_key{};
  VaultMeta new_meta;

  const auto fail = [&result](CoreStatus status) {
    result = status;
    return DBStatus::kError;  // Any non-kOk value rolls back.
  };

  const DBStatus db_status = db_.RunInTransaction([&]() -> DBStatus {
    const auto meta = db_.GetMeta();
    if (!meta.has_value()) {
      return fail(FromDBStatus(meta.error()));
    }

    if (!DeriveKey(current_password, *meta, old_key) ||
        !CheckVerifier(old_key, *meta)) {
      return fail(CoreStatus::kWrongPassword);
    }

    new_meta = *meta;
    new_meta.kdf_params = kdf_params.value_or(meta->kdf_params);
    new_meta.key_epoch = meta->key_epoch + 1;
    if (!Crypto::RandomBytes(new_meta.kdf_salt)) {
      return fail(CoreStatus::kError);
    }
    if (!DeriveKey(new_password, new_meta, new_key)) {
      return fail(CoreStatus::kInvalidArgument);
    }
    if (!SealVerifier(new_key, new_meta)) {
      return fail(CoreStatus::kError);
    }

    auto accounts = db_.GetAllAccounts();
    if (!accounts.has_value()) {
      return fail(FromDBStatus(accounts.error()));
    }
    for (auto& [id, binary] : *accounts) {
      AccountPlain plain;
      if (!Decode(old_key, binary, plain)) {
        return fail(CoreStatus::kCorrupted);
      }
      const auto uid = binary.uid;
      if (!Encode(new_key, plain, uid, binary)) {
        return fail(CoreStatus::kError);
      }
      const DBStatus update = db_.UpdateAccount(id, binary);
      if (update != DBStatus::kOk) {
        return fail(FromDBStatus(update));
      }
    }

    const DBStatus update_meta = db_.UpdateMeta(new_meta);
    if (update_meta != DBStatus::kOk) {
      return fail(FromDBStatus(update_meta));
    }
    return DBStatus::kOk;
  });

  Crypto::SecureClear(old_key);
  if (db_status != DBStatus::kOk) {
    Crypto::SecureClear(new_key);
    return result != CoreStatus::kOk ? result : FromDBStatus(db_status);
  }

  master_key_ = new_key;
  Crypto::SecureClear(new_key);
  key_epoch_ = new_meta.key_epoch;
  ClearIndex();
  ClearAccount();
  return CoreStatus::kOk;
}

bool Core::DeriveKey(std::string_view password, const VaultMeta& meta,
                     Key& key) {
  return Crypto::Kdf(AsBytes(password), meta.kdf_salt, meta.kdf_params, key);
}

bool Core::SealVerifier(const Key& key, VaultMeta& meta) {
  // The verifier plaintext is a fixed all-zero block: AES-GCM's tag is what
  // proves the key right, and the zero check in CheckVerifier is a second,
  // independent guard.
  meta.verifier.fill(std::byte{0});
  if (!Crypto::RandomBytes(meta.verifier_iv)) {
    return false;
  }
  return Crypto::Encrypt(key.data(), meta.verifier_iv.data(),
                         AsBytes(kVerifierAad), meta.verifier,
                         meta.verifier_tag.data());
}

bool Core::CheckVerifier(const Key& key, const VaultMeta& meta) {
  std::array<std::byte, VaultMeta::kVerifierSize> block = meta.verifier;
  const bool ok =
      Crypto::Decrypt(key.data(), meta.verifier_iv.data(),
                      AsBytes(kVerifierAad), block, meta.verifier_tag.data());
  return ok && std::ranges::all_of(
                   block, [](std::byte b) { return b == std::byte{0}; });
}

void Core::Close() noexcept {
  db_.Close();
  Crypto::SecureClear(std::span(master_key_));
  has_key_ = false;
  key_epoch_ = 0;
  ClearIndex();
  ClearAccount();
}

bool Core::IsOpen() const noexcept {
  return has_key_ && db_.IsOpen();
}

bool Core::Encode(const Key& key, const AccountPlain& account,
                  const std::array<std::byte, AccountSize::kUid>& uid,
                  AccountBinary& out) {
  out.SetZero();
  out.uid = uid;

  std::memcpy(out.description.data(), account.description.data(),
              out.description.size());

  const AccountView view(out);
  std::memcpy(view.email.data(), account.email.data(), view.email.size());
  std::memcpy(view.user_name.data(), account.username.data(),
              view.user_name.size());
  std::memcpy(view.password.data(), account.password.data(),
              view.password.size());
  std::memcpy(view.extra.data(), account.extra.data(), view.extra.size());

  // AES-GCM requires the (key, IV) pair to never repeat, so every encryption
  // draws brand new IVs from the OS CSPRNG rather than reusing or deriving
  // them.
  if (!Crypto::RandomBytes(out.iv_description) ||
      !Crypto::RandomBytes(out.iv_data)) {
    out.SetZero();
    return false;
  }

  const bool description_ok =
      Crypto::Encrypt(key.data(), out.iv_description.data(),
                      AccountAad(kDescriptionAad, out.uid),
                      std::span(out.description), out.tag_description.data());
  const bool data_ok = Crypto::Encrypt(
      key.data(), out.iv_data.data(), AccountAad(kDataAad, out.uid),
      std::span(out.data), out.tag_data.data());
  return description_ok && data_ok;
}

bool Core::Decode(const Key& key, AccountBinary& binary, AccountPlain& out) {
  const bool description_ok = Crypto::Decrypt(
      key.data(), binary.iv_description.data(),
      AccountAad(kDescriptionAad, binary.uid), std::span(binary.description),
      binary.tag_description.data());
  const bool data_ok = Crypto::Decrypt(
      key.data(), binary.iv_data.data(), AccountAad(kDataAad, binary.uid),
      std::span(binary.data), binary.tag_data.data());
  if (!description_ok || !data_ok) {
    return false;
  }

  std::memcpy(out.description.data(), binary.description.data(),
              out.description.size());

  const AccountView view(binary);
  std::memcpy(out.email.data(), view.email.data(), out.email.size());
  std::memcpy(out.username.data(), view.user_name.data(), out.username.size());
  std::memcpy(out.password.data(), view.password.data(), out.password.size());
  std::memcpy(out.extra.data(), view.extra.data(), out.extra.size());
  return true;
}

bool Core::DecodeDescription(AccountIndex& index, AccountEntry& out) const {
  const bool ok = Crypto::Decrypt(
      master_key_.data(), index.iv_description.data(),
      AccountAad(kDescriptionAad, index.uid), std::span(index.description),
      index.tag_description.data());
  if (!ok) {
    return false;
  }

  out.id = index.id;
  out.position = index.position;
  std::memcpy(out.description.data(), index.description.data(),
              out.description.size());
  return true;
}

CoreStatus Core::AddAccount(const AccountPlain& account, std::int64_t* out_id) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  std::array<std::byte, AccountSize::kUid> uid{};
  if (!Crypto::RandomBytes(uid)) {
    return CoreStatus::kError;
  }

  AccountBinary binary;
  if (!Encode(master_key_, account, uid, binary)) {
    return CoreStatus::kError;
  }

  const CoreStatus status = FromDBStatus(db_.AddAccount(binary, out_id));
  if (status == CoreStatus::kOk) {
    ClearIndex();
  }
  return status;
}

CoreStatus Core::EditAccount(std::int64_t id, const AccountPlain& account) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  // Keep the account's uid: it identifies the account, not its contents.
  const auto current = db_.GetAccount(id);
  if (!current.has_value()) {
    return FromDBStatus(current.error());
  }

  AccountBinary binary;
  if (!Encode(master_key_, account, current->uid, binary)) {
    return CoreStatus::kError;
  }

  const CoreStatus status = FromDBStatus(db_.UpdateAccount(id, binary));
  if (status == CoreStatus::kOk) {
    ClearIndex();
    if (id == loaded_id_) {
      ClearAccount();
    }
  }
  return status;
}

CoreStatus Core::RemoveAccount(std::int64_t id) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  const CoreStatus status = FromDBStatus(db_.DeleteAccount(id));
  if (status == CoreStatus::kOk) {
    ClearIndex();
    ClearAccount();
  }
  return status;
}

CoreStatus Core::ReorderAccount(std::int64_t id, std::int64_t to_position) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  const CoreStatus status = FromDBStatus(db_.MoveAccount(id, to_position));
  if (status == CoreStatus::kOk) {
    ClearIndex();
    ClearAccount();
  }
  return status;
}

CoreStatus Core::LoadIndex() {
  index_.clear();
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  auto raw = db_.GetAllIndex();
  if (!raw.has_value()) {
    return FromDBStatus(raw.error());
  }
  index_.reserve(raw->size());
  for (AccountIndex& entry : *raw) {
    AccountEntry decoded;
    if (DecodeDescription(entry, decoded)) {
      index_.push_back(decoded);
    }
  }
  return CoreStatus::kOk;
}

const std::vector<AccountEntry>& Core::Index() const noexcept {
  return index_;
}

void Core::ClearIndex() noexcept {
  index_.clear();
}

std::vector<std::size_t> Core::SearchIndex(std::string_view query) const {
  const auto lower = [](char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  };
  std::vector<std::size_t> matches;
  for (std::size_t i = 0; i < index_.size(); ++i) {
    const std::string_view description = GetAccountField(index_[i].description);
    const auto found = std::ranges::search(
        description, query,
        [&](char a, char b) { return lower(a) == lower(b); });
    if (query.empty() || !found.empty()) {
      matches.push_back(i);
    }
  }
  return matches;
}

std::expected<std::size_t, CoreStatus> Core::Count() {
  if (!IsOpen()) {
    return std::unexpected(CoreStatus::kLocked);
  }
  const auto count = db_.Count();
  if (!count.has_value()) {
    return std::unexpected(FromDBStatus(count.error()));
  }
  return *count;
}

CoreStatus Core::Compact() {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }
  return FromDBStatus(db_.Compact());
}

std::expected<const AccountPlain*, CoreStatus> Core::GetAccount(
    std::int64_t id) {
  ClearAccount();
  if (!IsOpen()) {
    return std::unexpected(CoreStatus::kLocked);
  }

  auto binary = db_.GetAccount(id);
  if (!binary.has_value()) {
    return std::unexpected(FromDBStatus(binary.error()));
  }

  loaded_account_.emplace();
  if (!Decode(master_key_, *binary, *loaded_account_)) {
    ClearAccount();
    return std::unexpected(CoreStatus::kCorrupted);
  }

  loaded_id_ = id;
  return &(*loaded_account_);
}

const AccountPlain* Core::CurrentAccount() const noexcept {
  return loaded_account_.has_value() ? &(*loaded_account_) : nullptr;
}

void Core::ClearAccount() noexcept {
  loaded_account_.reset();
  loaded_id_ = -1;
}
