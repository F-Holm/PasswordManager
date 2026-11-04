#include "core/core.h"

#include <cstring>
#include <optional>
#include <random>
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

// Fills `out` with fresh OS entropy. AES-GCM requires the (key, IV) pair to
// never repeat, so every encryption draws a brand new IV rather than reusing
// or deriving one.
void FillRandom(std::span<std::byte> out) noexcept {
  static std::random_device device;
  std::uniform_int_distribution<int> byte_dist(0, 255);
  for (std::byte& b : out) {
    b = static_cast<std::byte>(byte_dist(device));
  }
}

}  // namespace

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

bool Core::Open(std::string_view db_path,
                const std::array<std::byte, Crypto::Size::kKey>& master_key) {
  Close();

  if (!db_.Open(db_path)) {
    return false;
  }

  master_key_ = master_key;
  has_key_ = true;
  return true;
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

  FillRandom(out.iv_description);
  FillRandom(out.iv_data);

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

bool Core::AddAccount(const AccountPlain& account) {
  if (!IsOpen()) {
    return false;
  }

  AccountBinary binary;
  if (!Encode(account, binary)) {
    return false;
  }

  if (!db_.AddAccount(binary)) {
    return false;
  }

  ClearIndex();
  return true;
}

bool Core::EditAccount(std::int64_t position, const AccountPlain& account) {
  if (!IsOpen() || position < 0) {
    return false;
  }

  AccountBinary binary;
  if (!Encode(account, binary)) {
    return false;
  }

  if (!db_.UpdateAccount(position, binary)) {
    return false;
  }

  ClearIndex();
  if (position == loaded_position_) {
    ClearAccount();
  }
  return true;
}

bool Core::RemoveAccount(std::int64_t position) {
  if (!IsOpen() || position < 0) {
    return false;
  }

  if (!db_.DeleteAccount(position)) {
    return false;
  }

  ClearIndex();
  ClearAccount();
  return true;
}

bool Core::ReorderAccount(std::int64_t from_position,
                          std::int64_t to_position) {
  if (!IsOpen()) {
    return false;
  }

  if (!db_.MoveAccount(from_position, to_position)) {
    return false;
  }

  ClearIndex();
  ClearAccount();
  return true;
}

const std::vector<AccountEntry>& Core::GetAllIndex() {
  index_.clear();
  if (!IsOpen()) {
    return index_;
  }

  std::vector<AccountIndex> raw = db_.GetAllIndex();
  index_.reserve(raw.size());
  for (AccountIndex& entry : raw) {
    AccountEntry decoded;
    if (DecodeDescription(entry, decoded)) {
      index_.push_back(decoded);
    }
  }
  return index_;
}

const std::vector<AccountEntry>& Core::IndexCache() const noexcept {
  return index_;
}

void Core::ClearIndex() noexcept {
  index_.clear();
}

const AccountPlain* Core::GetAccount(std::int64_t position) {
  ClearAccount();
  if (!IsOpen() || position < 0) {
    return nullptr;
  }

  std::optional<AccountBinary> binary = db_.GetAccount(position);
  if (!binary.has_value()) {
    return nullptr;
  }

  loaded_account_.emplace();
  if (!Decode(*binary, *loaded_account_)) {
    ClearAccount();
    return nullptr;
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
