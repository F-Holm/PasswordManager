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
//
// Several programs may use the same vault at once, each with its own
// session. If another program changes the master password, this session's
// next read or write returns kCoreLocked (writing nothing) and the session
// is closed: unlock it again with the new password.
#ifdef __cplusplus
extern "C" {
#endif

// This header must stay valid C, so it keeps typedef'd structs, C arrays and
// plain enums that C++-only checks would rather see modernized.
// NOLINTBEGIN(modernize-use-using,modernize-avoid-c-arrays,performance-enum-size)

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
  // Revision the account was read at (filled in by CoreGetAccount, ignored
  // by CoreAddAccount); pass it back to CoreEditAccount.
  int64_t revision;
  char description[kCoreDescriptionSize];
  char email[kCoreEmailSize];
  char username[kCoreUsernameSize];
  char password[kCorePasswordSize];
  char extra[kCoreExtraSize];
} CoreAccount;

// A single entry of the account index: the account's stable id (used to
// address it in every call), its position in the list and its decrypted
// description.
typedef struct CoreAccountEntry {
  int64_t id;
  int64_t position;
  int64_t revision;
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

// Accounts are addressed by their stable id (CoreAccountEntry::id), which
// never changes and is never reused.

// Encrypts and appends `account` as a new entry and writes its id to
// *out_id unless `out_id` is NULL.
CoreResult CoreAddAccount(CoreSession* session, const CoreAccount* account,
                          int64_t* out_id);
// Passed as `expected_revision` to CoreEditAccount to skip the check.
enum { kCoreAnyRevision = -1 };

// Encrypts and overwrites the account `id`, provided it is still at
// `expected_revision` (normally account->revision as read by
// CoreGetAccount). kCoreConflict, with nothing written, if someone else
// changed it in the meantime.
CoreResult CoreEditAccount(CoreSession* session, int64_t id,
                           int64_t expected_revision,
                           const CoreAccount* account);
// Deletes the account `id`, shifting later positions down.
CoreResult CoreRemoveAccount(CoreSession* session, int64_t id);
// Moves the account `id` to `to_position` (0-based).
CoreResult CoreReorderAccount(CoreSession* session, int64_t id,
                              int64_t to_position);

// Reloads the full account index and writes its entry count to *out_count.
// Individual entries are then read with CoreGetIndexEntry.
CoreResult CoreLoadIndex(CoreSession* session, size_t* out_count);
// Copies the `list_index`-th (0-based) loaded index entry into *out_entry.
// kCoreNotFound if `list_index` is out of range.
CoreResult CoreGetIndexEntry(const CoreSession* session, size_t list_index,
                             CoreAccountEntry* out_entry);
void CoreClearIndex(CoreSession* session);
// Finds the loaded index entries whose description contains the
// `query_size` bytes at `query`, ignoring ASCII case (an empty query matches
// everything). Writes the total number of matches to *out_count and the
// first min(*out_count, capacity) matching list indices (as accepted by
// CoreGetIndexEntry) to `out_indices`, which may be NULL if capacity is 0.
CoreResult CoreSearchIndex(const CoreSession* session, const char* query,
                           size_t query_size, size_t* out_indices,
                           size_t capacity, size_t* out_count);

// Writes to *out_changed whether another program changed the vault since
// it was unlocked, the index was last loaded or this was last called. When
// it did, the loaded index and account are cleared: call CoreLoadIndex
// again. Cheap enough to poll.
CoreResult CoreCheckExternalChanges(CoreSession* session, bool* out_changed);

// Writes the number of accounts in the vault to *out_count.
CoreResult CoreCount(CoreSession* session, size_t* out_count);
// Rebuilds the vault file to reclaim the space of deleted accounts.
CoreResult CoreCompact(CoreSession* session);

// Decrypts the account `id` and copies it into *out_account, replacing
// whatever account was previously loaded in this session.
CoreResult CoreGetAccount(CoreSession* session, int64_t id,
                          CoreAccount* out_account);
void CoreClearAccount(CoreSession* session);

// NOLINTEND(modernize-use-using,modernize-avoid-c-arrays,performance-enum-size)

#ifdef __cplusplus
}  // extern "C"
#endif
