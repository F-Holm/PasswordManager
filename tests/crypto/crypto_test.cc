#include "crypto/crypto.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {
std::span<const std::byte> ToByteSpan(const std::string_view s) {
  return {reinterpret_cast<const std::byte*>(s.data()), s.size()};
}

// --- Hashing Tests ---

TEST(CryptoTest, Hash256ReturnsTrueAndPopulatesHash) {
  std::string_view data = "Hello, World!";
  std::byte hash[Crypto::Size::kSha256];

  EXPECT_TRUE(Crypto::Hash256(ToByteSpan(data), hash));
}

TEST(CryptoTest, Hash512ReturnsTrueAndPopulatesHash) {
  std::string_view data = "Hello, World!";
  std::byte hash[Crypto::Size::kSha512];

  EXPECT_TRUE(Crypto::Hash512(ToByteSpan(data), hash));
}

TEST(CryptoTest, Hash256ProducesUniqueOutputs) {
  constexpr int kNumBuffers = 100;
  std::set<std::vector<std::byte>> seen_hashes;

  for (int i = 0; i < kNumBuffers; ++i) {
    std::string input = "Data " + std::to_string(i);
    auto input_span = std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(input.data()), input.size());

    std::byte hash_out[Crypto::Size::kSha256];
    ASSERT_TRUE(Crypto::Hash256(input_span, hash_out))
        << "Hash256 failed at iteration " << i;

    std::vector<std::byte> hash_vec(hash_out, hash_out + Crypto::Size::kSha256);

    auto [iterator, inserted] = seen_hashes.insert(hash_vec);

    EXPECT_TRUE(inserted) << "Collision detected! Hash for '" << input
                          << "' is not unique.";
  }

  EXPECT_EQ(seen_hashes.size(), kNumBuffers);
}

TEST(CryptoTest, Hash512ProducesUniqueOutputs) {
  constexpr int kNumBuffers = 100;
  std::set<std::vector<std::byte>> seen_hashes;

  for (int i = 0; i < kNumBuffers; ++i) {
    std::string input = "Data " + std::to_string(i);
    auto input_span = std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(input.data()), input.size());

    std::byte hash_out[Crypto::Size::kSha512];
    ASSERT_TRUE(Crypto::Hash256(input_span, hash_out))
        << "Hash512 failed at iteration " << i;

    std::vector<std::byte> hash_vec(hash_out, hash_out + Crypto::Size::kSha512);

    auto [iterator, inserted] = seen_hashes.insert(hash_vec);

    EXPECT_TRUE(inserted) << "Collision detected! Hash for '" << input
                          << "' is not unique.";
  }

  EXPECT_EQ(seen_hashes.size(), kNumBuffers);
}

// --- Encryption/Decryption Tests ---

TEST(CryptoTest, EncryptionDecryptionRoundTrip) {
  std::byte key[32] = {std::byte{0x01}};
  std::byte iv[12] = {std::byte{0x02}};
  std::byte tag[16];

  std::string original_text = "Secret Message";
  std::vector<std::byte> buffer(original_text.size());
  std::memcpy(buffer.data(), original_text.data(), original_text.size());

  std::string_view aad_str = "Header Data";
  auto aad = ToByteSpan(aad_str);

  EXPECT_TRUE(Crypto::Encrypt(key, iv, aad, buffer, tag));

  EXPECT_NE(0, std::memcmp(buffer.data(), original_text.data(), buffer.size()));

  EXPECT_TRUE(Crypto::Decrypt(key, iv, aad, buffer, tag));

  std::string decrypted_text(reinterpret_cast<char*>(buffer.data()),
                             buffer.size());
  EXPECT_EQ(original_text, decrypted_text);
}

TEST(CryptoTest, DecryptionFailsWithWrongTag) {
  std::byte key[Crypto::Size::kKey] = {std::byte{0}};
  std::byte iv[Crypto::Size::kIv] = {std::byte{0}};
  std::byte tag[Crypto::Size::kTag];
  std::byte wrong_tag[Crypto::Size::kTag] = {std::byte{0xFF}};

  std::vector<std::byte> buffer(10, std::byte{0xAA});

  Crypto::Encrypt(key, iv, {}, buffer, tag);

  EXPECT_FALSE(Crypto::Decrypt(key, iv, {}, buffer, wrong_tag));
}

// --- KDF Tests ---

TEST(KDFTest, GeneratesConsistentKeyFromSamePassword) {
  auto password = ToByteSpan("my_secure_password");
  auto salt = ToByteSpan("random_salt");

  std::byte key1[32];
  std::byte key2[32];

  EXPECT_TRUE(Crypto::Kdf(password, salt, key1));
  EXPECT_TRUE(Crypto::Kdf(password, salt, key2));

  EXPECT_EQ(0, std::memcmp(key1, key2, 32));
}

TEST(KDFTest, HandlesVariousPasswordLengths) {
  std::byte salt[16] = {std::byte{0x12}};
  std::byte output_key[32];

  std::vector<size_t> sizes = {1, 16, 32, 64, 128};

  for (size_t size : sizes) {
    std::vector<std::byte> password(size, std::byte{0x41});
    std::span<const std::byte> pwd_span(password.data(), password.size());
    std::span<const std::byte> salt_span(salt, sizeof(salt));
    std::span<std::byte> out_span(output_key, sizeof(output_key));

    EXPECT_TRUE(Crypto::Kdf(pwd_span, salt_span, out_span))
        << "KDF failed for password size: " << size;
  }
}

TEST(KDFTest, DifferentInputsProduceDifferentOutputs) {
  constexpr int kIterations = 50;
  std::set<std::vector<std::byte>> seen_keys;

  std::byte salt[16] = {std::byte{0x00}};
  std::span<const std::byte> salt_span(salt, sizeof(salt));

  for (int i = 0; i < kIterations; ++i) {
    std::string password = "password_number_" + std::to_string(i);
    auto pwd_span = std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(password.data()), password.size());

    std::byte out_key[32];
    std::span<std::byte> out_span(out_key, sizeof(out_key));

    ASSERT_TRUE(Crypto::Kdf(pwd_span, salt_span, out_span));

    std::vector<std::byte> key_vec(out_key, out_key + 32);
    auto [unused_it, inserted] = seen_keys.insert(key_vec);

    EXPECT_TRUE(inserted) << "Collision detected in KDF output for: "
                          << password;
  }

  EXPECT_EQ(seen_keys.size(), kIterations);
}

TEST(KDFTest, SamePasswordDifferentSaltProducesDifferentOutputs) {
  std::string password = "constant_password";
  auto pwd_span = std::span<const std::byte>(
      reinterpret_cast<const std::byte*>(password.data()), password.size());

  std::byte key_a[32];
  std::byte key_b[32];

  std::byte salt_a[16] = {std::byte{0x01}};
  std::byte salt_b[16] = {std::byte{0x02}};

  EXPECT_TRUE(Crypto::Kdf(pwd_span, {salt_a, 16}, {key_a, 32}));
  EXPECT_TRUE(Crypto::Kdf(pwd_span, {salt_b, 16}, {key_b, 32}));

  EXPECT_NE(0, std::memcmp(key_a, key_b, 32));
}

// --- SecureClear Tests ---

TEST(SecureClearTest, ZerosOutBuffer) {
  std::vector<std::byte> buffer = {std::byte{0x01}, std::byte{0x02},
                                   std::byte{0x03}};

  EXPECT_TRUE(Crypto::SecureClear(buffer));

  for (auto b : buffer) {
    EXPECT_EQ(b, std::byte{0});
  }
}

TEST(SecureClearTest, HandlesNullBuffer) {
  std::span<std::byte> null_span;
  EXPECT_EQ(null_span.data(), nullptr);
  EXPECT_EQ(null_span.size(), 0);
  EXPECT_FALSE(Crypto::SecureClear(null_span));
}

TEST(SecureClearTest, HandlesEmptyBuffer) {
  std::byte dummy;
  std::span<std::byte> empty_span(&dummy, 0);
  EXPECT_NE(empty_span.data(), nullptr);
  EXPECT_EQ(empty_span.size(), 0);
  EXPECT_TRUE(Crypto::SecureClear(empty_span));
}

}  // namespace
