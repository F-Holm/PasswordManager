#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "crypto/crypto.h"
#include "db/db.h"
#include "types/account_binary.h"
#include "types/account_index.h"
#include "types/account_size.h"

// Plaintext view of an account, as exchanged with callers of Core. Every
// field is a fixed-capacity, NUL-padded buffer sized to match AccountSize,
// so it can be encrypted into (or decrypted out of) an AccountBinary without
// heap allocation.
struct AccountPlain {
  std::array<char, AccountSize::kDescription> description{};
  std::array<char, AccountSize::kEmail> email{};
  std::array<char, AccountSize::kUsername> username{};
  std::array<char, AccountSize::kPassword> password{};
  std::array<char, AccountSize::kExtra> extra{};

  void SetZero() noexcept;
  ~AccountPlain() noexcept;
};

// Plaintext view of a single index entry: the account's storage position
// plus its decrypted description, used to render account lists without
// decrypting the (much larger) account data.
struct AccountEntry {
  std::int64_t position = -1;
  std::array<char, AccountSize::kDescription> description{};

  void SetZero() noexcept;
  ~AccountEntry() noexcept;
};

// Copies `value` into `field`, NUL-padding the remainder. Returns false
// without modifying `field` if `value` does not fit.
bool SetAccountField(std::span<char> field, std::string_view value) noexcept;

// Returns a view of `field` up to its first NUL byte (or the whole field if
// it is not NUL-terminated).
std::string_view GetAccountField(std::span<const char> field) noexcept;

// Core is the single entry point for every action the password manager can
// perform on a vault: unlocking it, and adding, editing, removing and
// reordering the accounts it stores, plus reading them back.
//
// A Core instance may hold the full account index (position + description)
// in memory at once, but only ever decrypts one full account at a time; any
// operation that can invalidate the loaded index or the loaded account
// clears the corresponding cache instead of leaving stale plaintext around.
//
// Core is not thread-safe: callers must serialize access to a single
// instance themselves.
class Core {
 public:
  Core() noexcept;
  ~Core();

  Core(const Core&) = delete;
  Core& operator=(const Core&) = delete;
  Core(Core&&) = delete;
  Core& operator=(Core&&) = delete;

  // Opens the vault database at `db_path` using an already-derived
  // `master_key` (e.g. via Crypto::Kdf). Replaces any previously open vault.
  bool Open(std::string_view db_path,
            const std::array<std::byte, Crypto::Size::kKey>& master_key);
  // Closes the vault, wipes the master key and every cached plaintext.
  void Close() noexcept;
  [[nodiscard]] bool IsOpen() const noexcept;

  // Encrypts and appends `account` as a new entry. Invalidates the cached
  // index.
  bool AddAccount(const AccountPlain& account);
  // Encrypts and overwrites the account at `position`. Invalidates the
  // cached index, and clears the loaded account if it was `position`.
  bool EditAccount(std::int64_t position, const AccountPlain& account);
  // Deletes the account at `position`, shifting later positions down.
  // Invalidates the cached index and the loaded account.
  bool RemoveAccount(std::int64_t position);
  // Moves the account at `from_position` to `to_position`, shifting the
  // accounts in between. Invalidates the cached index and the loaded
  // account.
  bool ReorderAccount(std::int64_t from_position, std::int64_t to_position);

  // Reloads and decrypts every index entry from the vault, caches it and
  // returns a reference to the cache. The reference is invalidated by the
  // next call to GetAllIndex or ClearIndex.
  const std::vector<AccountEntry>& GetAllIndex();
  // Returns the index cache as it stands, without refreshing it from the
  // vault. Useful to read entries back after a prior GetAllIndex() call.
  [[nodiscard]] const std::vector<AccountEntry>& IndexCache() const noexcept;
  void ClearIndex() noexcept;

  // Decrypts the account at `position`, replacing whatever account was
  // previously loaded, and returns a pointer to it (nullptr on failure).
  // The pointer is invalidated by the next call to GetAccount or
  // ClearAccount, or by any mutating operation.
  const AccountPlain* GetAccount(std::int64_t position);
  [[nodiscard]] const AccountPlain* CurrentAccount() const noexcept;
  void ClearAccount() noexcept;

 private:
  DB db_;
  std::array<std::byte, Crypto::Size::kKey> master_key_{};
  bool has_key_ = false;

  std::vector<AccountEntry> index_;
  std::optional<AccountPlain> loaded_account_;
  std::int64_t loaded_position_ = -1;

  bool Encode(const AccountPlain& account, AccountBinary& out) const;
  bool Decode(AccountBinary& binary, AccountPlain& out) const;
  bool DecodeDescription(AccountIndex& index, AccountEntry& out) const;
};
