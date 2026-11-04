#pragma once

#include <cstddef>
#include <cstdint>

#include "types/account_size.h"

// Generic C ABI bridging Core to any host language with a C FFI (Rust, C,
// C#, Swift, ...). Every function is exception-safe: a C++ exception raised
// while servicing a call is caught internally and reported as a failure
// return instead of unwinding across the ABI boundary, which callers on the
// C side could not handle.
//
// CoreSession is opaque on purpose: callers only ever hold a pointer to it,
// created by CoreOpen and destroyed by CoreClose.
extern "C" {

// Field capacities, in bytes, mirrored from AccountSize/Crypto::Size so
// bindings can size their own buffers without depending on the C++ headers.
extern const std::size_t kCoreDescriptionSize;
extern const std::size_t kCoreEmailSize;
extern const std::size_t kCoreUsernameSize;
extern const std::size_t kCorePasswordSize;
extern const std::size_t kCoreExtraSize;
extern const std::size_t kCoreMasterKeySize;

struct CoreSession;

// Flat, fixed-size plaintext account record used at the C boundary. Every
// array is NUL-padded; unused trailing bytes are zero.
struct CoreAccount {
  char description[AccountSize::kDescription];
  char email[AccountSize::kEmail];
  char username[AccountSize::kUsername];
  char password[AccountSize::kPassword];
  char extra[AccountSize::kExtra];
};

// A single entry of the account index: its storage position plus its
// decrypted description.
struct CoreAccountEntry {
  std::int64_t position;
  char description[AccountSize::kDescription];
};

// Opens the vault database at `db_path` (a NUL-terminated path) using an
// already-derived master key of exactly kCoreMasterKeySize bytes. Returns a
// new session on success, or nullptr on failure (invalid key size, or the
// database could not be opened).
CoreSession* CoreOpen(const char* db_path, const std::uint8_t* master_key,
                      std::size_t master_key_size);
// Closes and frees `session`, wiping every secret it held. `session` may be
// nullptr, in which case this is a no-op.
void CoreClose(CoreSession* session);
bool CoreIsOpen(const CoreSession* session);

// Encrypts and appends `account` as a new entry.
bool CoreAddAccount(CoreSession* session, const CoreAccount* account);
// Encrypts and overwrites the account at `position`.
bool CoreEditAccount(CoreSession* session, std::int64_t position,
                     const CoreAccount* account);
// Deletes the account at `position`, shifting later positions down.
bool CoreRemoveAccount(CoreSession* session, std::int64_t position);
// Moves the account at `from_position` to `to_position`.
bool CoreReorderAccount(CoreSession* session, std::int64_t from_position,
                        std::int64_t to_position);

// Reloads the full account index and returns its entry count. Individual
// entries are then read with CoreGetIndexEntry.
std::size_t CoreLoadIndex(CoreSession* session);
// Copies the `list_index`-th (0-based) cached index entry into *out_entry.
// Returns false if `list_index` is out of range.
bool CoreGetIndexEntry(const CoreSession* session, std::size_t list_index,
                       CoreAccountEntry* out_entry);
void CoreClearIndex(CoreSession* session);

// Decrypts the account at `position` and copies it into *out_account,
// replacing whatever account was previously loaded in this session.
bool CoreGetAccount(CoreSession* session, std::int64_t position,
                    CoreAccount* out_account);
void CoreClearAccount(CoreSession* session);

}  // extern "C"
