#pragma once

#include <cstdint>
#include <string_view>

// Outcome of a Core operation. The numeric values are part of the C ABI
// (mirrored as kCore* constants in core/core_interface.h) and must never be
// renumbered.
enum class CoreStatus : std::int32_t {
  kOk = 0,
  // The requested account does not exist (any more).
  kNotFound = 1,
  // The account was modified by someone else since it was read.
  kConflict = 2,
  // Another program held the vault locked for too long. Nothing was
  // changed; the operation can be retried.
  kBusy = 3,
  // No vault is unlocked in this session (never opened, closed, or the
  // master password was changed elsewhere and it must be unlocked again).
  kLocked = 4,
  // The master password does not open this vault.
  kWrongPassword = 5,
  // An argument is out of range, malformed or missing.
  kInvalidArgument = 6,
  // A vault already exists where a new one was to be created.
  kAlreadyExists = 7,
  // Stored data failed authentication: the vault file is corrupted or was
  // tampered with.
  kCorrupted = 8,
  // Any other failure (I/O error, out of memory, ...).
  kError = 9,
};

// Stable, human-readable name of `status` (e.g. "not found").
std::string_view ToString(CoreStatus status) noexcept;
