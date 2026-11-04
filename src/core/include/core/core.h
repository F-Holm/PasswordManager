#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "core/core_status.h"
#include "crypto/crypto.h"
#include "db/db.h"
#include "types/account_binary.h"
#include "types/account_index.h"
#include "types/account_size.h"
#include "types/vault_meta.h"

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

  // Creates a new vault at `db_path` protected by `master_password` and
  // leaves it unlocked. A random salt is generated and the master key is
  // derived from the password with Argon2id using `kdf_params`, which are
  // stored in the vault. Replaces any previously open vault.
  //
  // kAlreadyExists if `db_path` already holds a vault (or accounts);
  // kInvalidArgument if the password is empty or `kdf_params` are invalid.
  CoreStatus Create(
      std::string_view db_path, std::string_view master_password,
      const Crypto::KdfParams& kdf_params = Crypto::kDefaultKdfParams);
  // Unlocks the existing vault at `db_path` with `master_password`.
  // Replaces any previously open vault.
  //
  // kNotFound if there is no vault at `db_path`; kWrongPassword if the
  // password does not open it; kCorrupted if its metadata is unusable.
  CoreStatus Unlock(std::string_view db_path, std::string_view master_password);
  // Closes the vault, wipes the master key and every cached plaintext.
  void Close() noexcept;
  [[nodiscard]] bool IsOpen() const noexcept;

  // Every operation below returns kLocked if no vault is open.

  // Encrypts and appends `account` as a new entry. Invalidates the cached
  // index.
  CoreStatus AddAccount(const AccountPlain& account);
  // Encrypts and overwrites the account at `position` (kNotFound if there is
  // none). Invalidates the cached index, and clears the loaded account if it
  // was `position`.
  CoreStatus EditAccount(std::int64_t position, const AccountPlain& account);
  // Deletes the account at `position`, shifting later positions down.
  // Invalidates the cached index and the loaded account.
  CoreStatus RemoveAccount(std::int64_t position);
  // Moves the account at `from_position` to `to_position`, shifting the
  // accounts in between. Invalidates the cached index and the loaded
  // account.
  CoreStatus ReorderAccount(std::int64_t from_position,
                            std::int64_t to_position);

  // Reloads and decrypts every index entry from the vault into the cache
  // returned by Index(). Entries that fail authentication are left out.
  CoreStatus LoadIndex();
  // The index as loaded by the last LoadIndex() call. The reference stays
  // valid for the lifetime of the Core; its contents are replaced by the
  // next LoadIndex() and cleared by ClearIndex() and by any mutation.
  [[nodiscard]] const std::vector<AccountEntry>& Index() const noexcept;
  void ClearIndex() noexcept;

  // Decrypts the account at `position`, replacing whatever account was
  // previously loaded, and returns a pointer to it. The pointer is
  // invalidated by the next call to GetAccount or ClearAccount, or by any
  // mutating operation. kCorrupted if the stored data fails authentication.
  std::expected<const AccountPlain*, CoreStatus> GetAccount(
      std::int64_t position);
  [[nodiscard]] const AccountPlain* CurrentAccount() const noexcept;
  void ClearAccount() noexcept;

 private:
  using Key = std::array<std::byte, Crypto::Size::kKey>;

  DB db_;
  Key master_key_{};
  bool has_key_ = false;
  std::int64_t key_epoch_ = 0;

  std::vector<AccountEntry> index_;
  std::optional<AccountPlain> loaded_account_;
  std::int64_t loaded_position_ = -1;

  // Derives the key for `meta` from `password` into `key`.
  static bool DeriveKey(std::string_view password, const VaultMeta& meta,
                        Key& key);
  // Fills meta.verifier* by encrypting the known verifier block with `key`.
  static bool SealVerifier(const Key& key, VaultMeta& meta);
  // True if `key` decrypts and authenticates meta.verifier.
  static bool CheckVerifier(const Key& key, const VaultMeta& meta);

  // Encrypts `account` into `out` under `uid` (the account's stable random
  // identifier, generated once by AddAccount).
  bool Encode(const AccountPlain& account,
              const std::array<std::byte, AccountSize::kUid>& uid,
              AccountBinary& out) const;
  bool Decode(AccountBinary& binary, AccountPlain& out) const;
  bool DecodeDescription(AccountIndex& index, AccountEntry& out) const;
};
