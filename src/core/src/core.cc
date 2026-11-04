#include "core/core.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>

#include "types/account_view.h"

namespace {

// Domain-separates the two independently-encrypted fields of an account so
// their ciphertexts (and authentication tags) can never be swapped for one
// another.
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

bool Core::Encode(const AccountPlain& account, AccountBinary& out) const {
  out.SetZero();

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

  const bool description_ok = Crypto::Encrypt(
      master_key_.data(), out.iv_description.data(), AsBytes(kDescriptionAad),
      std::span(out.description), out.tag_description.data());
  const bool data_ok =
      Crypto::Encrypt(master_key_.data(), out.iv_data.data(), AsBytes(kDataAad),
                      std::span(out.data), out.tag_data.data());
  return description_ok && data_ok;
}

bool Core::Decode(AccountBinary& binary, AccountPlain& out) const {
  const bool description_ok =
      Crypto::Decrypt(master_key_.data(), binary.iv_description.data(),
                      AsBytes(kDescriptionAad), std::span(binary.description),
                      binary.tag_description.data());
  const bool data_ok = Crypto::Decrypt(
      master_key_.data(), binary.iv_data.data(), AsBytes(kDataAad),
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
      master_key_.data(), index.iv_description.data(), AsBytes(kDescriptionAad),
      std::span(index.description), index.tag_description.data());
  if (!ok) {
    return false;
  }

  out.position = index.position;
  std::memcpy(out.description.data(), index.description.data(),
              out.description.size());
  return true;
}

CoreStatus Core::AddAccount(const AccountPlain& account) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  AccountBinary binary;
  if (!Encode(account, binary)) {
    return CoreStatus::kError;
  }

  const CoreStatus status = FromDBStatus(db_.AddAccount(binary));
  if (status == CoreStatus::kOk) {
    ClearIndex();
  }
  return status;
}

CoreStatus Core::EditAccount(std::int64_t position,
                             const AccountPlain& account) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }
  if (position < 0) {
    return CoreStatus::kNotFound;
  }

  AccountBinary binary;
  if (!Encode(account, binary)) {
    return CoreStatus::kError;
  }

  const CoreStatus status = FromDBStatus(db_.UpdateAccount(position, binary));
  if (status == CoreStatus::kOk) {
    ClearIndex();
    if (position == loaded_position_) {
      ClearAccount();
    }
  }
  return status;
}

CoreStatus Core::RemoveAccount(std::int64_t position) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }
  if (position < 0) {
    return CoreStatus::kNotFound;
  }

  const CoreStatus status = FromDBStatus(db_.DeleteAccount(position));
  if (status == CoreStatus::kOk) {
    ClearIndex();
    ClearAccount();
  }
  return status;
}

CoreStatus Core::ReorderAccount(std::int64_t from_position,
                                std::int64_t to_position) {
  if (!IsOpen()) {
    return CoreStatus::kLocked;
  }

  const CoreStatus status =
      FromDBStatus(db_.MoveAccount(from_position, to_position));
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

std::expected<const AccountPlain*, CoreStatus> Core::GetAccount(
    std::int64_t position) {
  ClearAccount();
  if (!IsOpen()) {
    return std::unexpected(CoreStatus::kLocked);
  }
  if (position < 0) {
    return std::unexpected(CoreStatus::kNotFound);
  }

  auto binary = db_.GetAccount(position);
  if (!binary.has_value()) {
    return std::unexpected(FromDBStatus(binary.error()));
  }

  loaded_account_.emplace();
  if (!Decode(*binary, *loaded_account_)) {
    ClearAccount();
    return std::unexpected(CoreStatus::kCorrupted);
  }

  loaded_position_ = position;
  return &(*loaded_account_);
}

const AccountPlain* Core::CurrentAccount() const noexcept {
  return loaded_account_.has_value() ? &(*loaded_account_) : nullptr;
}

void Core::ClearAccount() noexcept {
  loaded_account_.reset();
  loaded_position_ = -1;
}
