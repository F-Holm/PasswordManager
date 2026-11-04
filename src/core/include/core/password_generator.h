#pragma once

#include <cstddef>
#include <span>

#include "core/core_status.h"

// What a generated password may contain.
struct PasswordOptions {
  // Number of characters, from 1 to kMaxGeneratedPasswordLength.
  std::size_t length = 20;
  bool numbers = true;  // 0-9
  bool upper = true;    // A-Z
  bool lower = true;    // a-z
  // The 32 ASCII punctuation characters: !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
  bool symbols = true;
  // Leaves out characters that are easy to confuse when read or typed by
  // hand: 0 O o 1 l I | ` ' "
  bool exclude_ambiguous = false;
};

// Longest password that still fits, NUL-terminated, in an account's
// password field.
inline constexpr std::size_t kMaxGeneratedPasswordLength = 255;

// Writes a random password of `options.length` characters followed by a NUL
// into `out`, drawing from the OS CSPRNG without modulo bias. The password
// contains at least one character of every enabled class.
//
// kInvalidArgument (with `out` zeroed) if no class is enabled, the length is
// 0, above kMaxGeneratedPasswordLength or smaller than the number of
// enabled classes, or `out` cannot hold length + 1 characters.
CoreStatus GeneratePassword(const PasswordOptions& options,
                            std::span<char> out);
