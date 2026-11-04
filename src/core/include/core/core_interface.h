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
// created by CoreNew and destroyed by CoreFree.
//
// Every fallible function returns a CoreResult (kCoreOk on success); outputs
// are written through pointer arguments and left untouched on failure.
#ifdef __cplusplus
extern "C" {
#endif

// Field capacities, in bytes, so bindings can size their own buffers without
// depending on the C++ headers. core_interface.cc checks that they match
// AccountSize.
enum {
  kCoreDescriptionSize = 128,
  kCoreEmailSize = 256,
  kCoreUsernameSize = 128,
  kCorePasswordSize = 256,
  kCoreExtraSize = 1024
};

// Result codes. Same values as CoreStatus in core/core_status.h.
typedef int32_t CoreResult;
enum {
  kCoreOk = 0,
  kCoreNotFound = 1,
  kCoreConflict = 2,
  kCoreBusy = 3,
  kCoreLocked = 4,
  kCoreWrongPassword = 5,
  kCoreInvalidArgument = 6,
  kCoreAlreadyExists = 7,
  kCoreCorrupted = 8,
  kCoreError = 9
};

typedef struct CoreSession CoreSession;

// Argon2id cost parameters used to derive a new vault's master key:
// memory in KiB (at least 8 per lane), passes and lanes (at least 1 each).
typedef struct CoreKdfParams {
  uint32_t memory_kib;
  uint32_t iterations;
  uint32_t parallelism;
} CoreKdfParams;

// The parameters CoreCreate uses when given NULL (64 MiB, 3 passes, 4 lanes).
CoreKdfParams CoreDefaultKdfParams(void);

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

// What CoreGeneratePassword may put in a password (see PasswordOptions in
// core/password_generator.h). `length` is 1 to kCoreMaxGeneratedPasswordLength.
typedef struct CorePasswordOptions {
  size_t length;
  bool numbers;
  bool upper;
  bool lower;
  bool symbols;
  bool exclude_ambiguous;
} CorePasswordOptions;

enum { kCoreMaxGeneratedPasswordLength = kCorePasswordSize - 1 };

// Writes a random password plus a terminating NUL into the `out_size` bytes
// at `out`, with at least one character of every enabled class. Needs no
// session. kCoreInvalidArgument (with `out` zeroed when it is not NULL) if
// the options are unusable or `out` is too small.
CoreResult CoreGeneratePassword(const CorePasswordOptions* options, char* out,
                                size_t out_size);

// Stable, human-readable, NUL-terminated name of `result` (e.g. "busy").
// Never returns NULL.
const char* CoreResultName(CoreResult result);

// Creates an empty session with no vault open. Returns NULL only if memory
// is exhausted.
CoreSession* CoreNew(void);
// Closes any open vault, wipes every secret the session held and frees it.
// `session` may be NULL.
void CoreFree(CoreSession* session);

// Creates a new vault at `db_path` (a NUL-terminated path) protected by the
// `password_size` bytes at `password` (not necessarily NUL-terminated) and
// leaves it open in `session`. `kdf_params` may be NULL to use
// CoreDefaultKdfParams(). Replaces any vault already open in `session`.
//
// kCoreAlreadyExists if `db_path` already holds a vault; kCoreInvalidArgument
// if the password is empty or the parameters are invalid.
CoreResult CoreCreate(CoreSession* session, const char* db_path,
                      const char* password, size_t password_size,
                      const CoreKdfParams* kdf_params);
// Opens the existing vault at `db_path` with the master password. Replaces
// any vault already open in `session`.
//
// kCoreNotFound if there is no vault at `db_path`; kCoreWrongPassword if the
// password does not open it.
CoreResult CoreUnlock(CoreSession* session, const char* db_path,
                      const char* password, size_t password_size);
// Replaces the master password of the vault open in `session` (see
// Core::ChangeMasterPassword). `kdf_params` may be NULL to keep the vault's
// current parameters. On failure the vault is left exactly as it was.
//
// kCoreWrongPassword if `current_password` is wrong; kCoreInvalidArgument if
// the new password is empty or the parameters are invalid.
CoreResult CoreChangeMasterPassword(CoreSession* session,
                                    const char* current_password,
                                    size_t current_password_size,
                                    const char* new_password,
                                    size_t new_password_size,
                                    const CoreKdfParams* kdf_params);
// Closes the vault and wipes the master key and every cached plaintext; the
// session can be reused. `session` may be NULL.
void CoreClose(CoreSession* session);
bool CoreIsOpen(const CoreSession* session);

// Encrypts and appends `account` as a new entry.
CoreResult CoreAddAccount(CoreSession* session, const CoreAccount* account);
// Encrypts and overwrites the account at `position`.
CoreResult CoreEditAccount(CoreSession* session, int64_t position,
                           const CoreAccount* account);
// Deletes the account at `position`, shifting later positions down.
CoreResult CoreRemoveAccount(CoreSession* session, int64_t position);
// Moves the account at `from_position` to `to_position`.
CoreResult CoreReorderAccount(CoreSession* session, int64_t from_position,
                              int64_t to_position);

// Reloads the full account index and writes its entry count to *out_count.
// Individual entries are then read with CoreGetIndexEntry.
CoreResult CoreLoadIndex(CoreSession* session, size_t* out_count);
// Copies the `list_index`-th (0-based) loaded index entry into *out_entry.
// kCoreNotFound if `list_index` is out of range.
CoreResult CoreGetIndexEntry(const CoreSession* session, size_t list_index,
                             CoreAccountEntry* out_entry);
void CoreClearIndex(CoreSession* session);

// Decrypts the account at `position` and copies it into *out_account,
// replacing whatever account was previously loaded in this session.
CoreResult CoreGetAccount(CoreSession* session, int64_t position,
                          CoreAccount* out_account);
void CoreClearAccount(CoreSession* session);

#ifdef __cplusplus
}  // extern "C"
#endif
