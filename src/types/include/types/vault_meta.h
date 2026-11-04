#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "crypto/crypto.h"

// Per-vault metadata, stored once in the vault next to the accounts.
//
// Nothing here is secret: the salt and KDF parameters are needed to derive
// the master key from the master password, and `verifier` is a known block
// encrypted with that key, used to reject a wrong password before anything
// is read or written with it.
struct VaultMeta {
  // Bumped whenever the on-disk format changes incompatibly.
  static constexpr std::int64_t kCurrentVersion = 1;
  static constexpr std::size_t kVerifierSize = 32;

  std::int64_t version = kCurrentVersion;
  std::array<std::byte, Crypto::Size::kSalt> kdf_salt{};
  Crypto::KdfParams kdf_params = Crypto::kDefaultKdfParams;
  std::array<std::byte, kVerifierSize> verifier{};
  std::array<std::byte, Crypto::Size::kIv> verifier_iv{};
  std::array<std::byte, Crypto::Size::kTag> verifier_tag{};
  // Incremented every time the master password changes, so other sessions
  // on the same vault can tell that their key is stale.
  std::int64_t key_epoch = 0;

  friend bool operator==(const VaultMeta&, const VaultMeta&) = default;
};
