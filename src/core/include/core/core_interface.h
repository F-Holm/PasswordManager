#pragma once

/* NOLINTBEGIN(modernize-deprecated-headers,modernize-use-using) */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* NOLINTEND(modernize-deprecated-headers,modernize-use-using) */

// Generic C ABI bridging Core to any host language with a C FFI (Rust, C,
// C#, Swift, ...). This header is valid C (C99 or later) and C++.
//
// Every function is exception-safe: a C++ exception raised while servicing a
// call is caught internally and reported as a failure return instead of
// unwinding across the ABI boundary, which callers on the C side could not
// handle.
//
// CoreSession is opaque on purpose: callers only ever hold a pointer to it,
// created by CoreOpen and destroyed by CoreClose.
#ifdef __cplusplus
extern "C" {
#endif

// Field capacities, in bytes, so bindings can size their own buffers without
// depending on the C++ headers. core_interface.cc checks that they match
// AccountSize and Crypto::Size.
enum {
  kCoreDescriptionSize = 128,
  kCoreEmailSize = 256,
  kCoreUsernameSize = 128,
  kCorePasswordSize = 256,
  kCoreExtraSize = 1024,
  kCoreMasterKeySize = 32
};

typedef struct CoreSession CoreSession;

// Flat, fixed-size plaintext account record used at the C boundary. Every
// array is NUL-padded; unused trailing bytes are zero.
typedef struct CoreAccount {
  char description[kCoreDescriptionSize];
  char email[kCoreEmailSize];
  char username[kCoreUsernameSize];
  char password[kCorePasswordSize];
  char extra[kCoreExtraSize];
} CoreAccount;

// A single entry of the account index: its storage position plus its
// decrypted description.
typedef struct CoreAccountEntry {
  int64_t position;
  char description[kCoreDescriptionSize];
} CoreAccountEntry;

// Opens the vault database at `db_path` (a NUL-terminated path) using an
// already-derived master key of exactly kCoreMasterKeySize bytes. Returns a
// new session on success, or nullptr on failure (invalid key size, or the
// database could not be opened).
CoreSession* CoreOpen(const char* db_path, const uint8_t* master_key,
                      size_t master_key_size);
// Closes and frees `session`, wiping every secret it held. `session` may be
// nullptr, in which case this is a no-op.
void CoreClose(CoreSession* session);
bool CoreIsOpen(const CoreSession* session);

// Encrypts and appends `account` as a new entry.
bool CoreAddAccount(CoreSession* session, const CoreAccount* account);
// Encrypts and overwrites the account at `position`.
bool CoreEditAccount(CoreSession* session, int64_t position,
                     const CoreAccount* account);
// Deletes the account at `position`, shifting later positions down.
bool CoreRemoveAccount(CoreSession* session, int64_t position);
// Moves the account at `from_position` to `to_position`.
bool CoreReorderAccount(CoreSession* session, int64_t from_position,
                        int64_t to_position);

// Reloads the full account index and returns its entry count. Individual
// entries are then read with CoreGetIndexEntry.
size_t CoreLoadIndex(CoreSession* session);
// Copies the `list_index`-th (0-based) cached index entry into *out_entry.
// Returns false if `list_index` is out of range.
bool CoreGetIndexEntry(const CoreSession* session, size_t list_index,
                       CoreAccountEntry* out_entry);
void CoreClearIndex(CoreSession* session);

// Decrypts the account at `position` and copies it into *out_account,
// replacing whatever account was previously loaded in this session.
bool CoreGetAccount(CoreSession* session, int64_t position,
                    CoreAccount* out_account);
void CoreClearAccount(CoreSession* session);

#ifdef __cplusplus
}  // extern "C"
#endif
