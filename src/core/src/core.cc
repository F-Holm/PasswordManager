#include "core/core.h"

#include <cstring>
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

CoreStatus Core::Open(
    std::string_view db_path,
    const std::array<std::byte, Crypto::Size::kKey>& master_key) {
  Close();

  if (!db_.Open(db_path)) {
    return CoreStatus::kError;
  }

  master_key_ = master_key;
  has_key_ = true;
  return CoreStatus::kOk;
}

void Core::Close() noexcept {
  db_.Close();
  Crypto::SecureClear(std::span(master_key_));
  has_key_ = false;
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
