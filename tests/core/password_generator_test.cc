#include "core/password_generator.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include "core/core_interface.h"

namespace {

constexpr std::string_view kSymbols = R"(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)";
constexpr std::string_view kAmbiguous = R"(0Oo1lI|`'")";

bool IsSymbol(char c) {
  return kSymbols.find(c) != std::string_view::npos;
}

std::string Generate(const PasswordOptions& options) {
  std::array<char, kMaxGeneratedPasswordLength + 1> out{};
  EXPECT_EQ(GeneratePassword(options, out), CoreStatus::kOk);
  return {out.data()};
}

}  // namespace

TEST(PasswordGeneratorTest, ProducesRequestedLengthAndNulTerminates) {
  for (const std::size_t length : {1u, 4u, 20u, 64u, 255u}) {
    PasswordOptions options;
    options.length = length;
    options.numbers = options.upper = options.symbols = length >= 4;
    std::array<char, 300> out{};
    out.fill('X');
    ASSERT_EQ(GeneratePassword(options, out), CoreStatus::kOk) << length;
    EXPECT_EQ(std::strlen(out.data()), length);
  }
}

TEST(PasswordGeneratorTest, UsesOnlyEnabledClasses) {
  PasswordOptions options;
  options.length = 200;
  options.upper = options.symbols = false;
  for (int i = 0; i < 20; ++i) {
    for (const char c : Generate(options)) {
      EXPECT_TRUE(std::isdigit(static_cast<unsigned char>(c)) ||
                  std::islower(static_cast<unsigned char>(c)))
          << c;
    }
  }
}

TEST(PasswordGeneratorTest, ContainsEveryEnabledClass) {
  PasswordOptions options;
  options.length = 4;  // Exactly one slot per class.
  for (int i = 0; i < 200; ++i) {
    const std::string password = Generate(options);
    const auto has = [&](auto predicate) {
      return std::ranges::any_of(password, [&](char c) {
        return predicate(static_cast<unsigned char>(c));
      });
    };
    EXPECT_TRUE(has([](unsigned char c) { return std::isdigit(c) != 0; }));
    EXPECT_TRUE(has([](unsigned char c) { return std::isupper(c) != 0; }));
    EXPECT_TRUE(has([](unsigned char c) { return std::islower(c) != 0; }));
    EXPECT_TRUE(has([](unsigned char c) { return IsSymbol(c); }));
  }
}

TEST(PasswordGeneratorTest, ExcludesAmbiguousCharacters) {
  PasswordOptions options;
  options.length = 255;
  options.exclude_ambiguous = true;
  for (int i = 0; i < 20; ++i) {
    for (const char c : Generate(options)) {
      EXPECT_EQ(kAmbiguous.find(c), std::string_view::npos) << c;
    }
  }
}

TEST(PasswordGeneratorTest, EveryCharacterOfTheAlphabetShowsUp) {
  // 94 printable characters; ~25k draws make missing any one of them
  // astronomically unlikely unless the generator is broken or biased.
  PasswordOptions options;
  options.length = 255;
  std::map<char, int> counts;
  for (int i = 0; i < 100; ++i) {
    for (const char c : Generate(options)) {
      ++counts[c];
    }
  }
  EXPECT_EQ(counts.size(), 10u + 26u + 26u + 32u);
  for (const auto& [c, count] : counts) {
    // Expected ~271 per character; a fair generator stays far from these.
    EXPECT_GT(count, 150) << c;
    EXPECT_LT(count, 450) << c;
  }
}

TEST(PasswordGeneratorTest, PasswordsDiffer) {
  std::set<std::string> seen;
  for (int i = 0; i < 100; ++i) {
    seen.insert(Generate(PasswordOptions{}));
  }
  EXPECT_EQ(seen.size(), 100u);
}

TEST(PasswordGeneratorTest, RejectsUnusableOptions) {
  std::array<char, 300> out{};
  const auto expect_invalid = [&](const PasswordOptions& options,
                                  std::span<char> buffer) {
    std::ranges::fill(buffer, 'X');
    EXPECT_EQ(GeneratePassword(options, buffer), CoreStatus::kInvalidArgument);
    EXPECT_TRUE(std::ranges::all_of(buffer, [](char c) { return c == '\0'; }));
  };

  PasswordOptions none;
  none.numbers = none.upper = none.lower = none.symbols = false;
  expect_invalid(none, out);

  PasswordOptions zero;
  zero.length = 0;
  expect_invalid(zero, out);

  PasswordOptions too_long;
  too_long.length = kMaxGeneratedPasswordLength + 1;
  expect_invalid(too_long, out);

  PasswordOptions fewer_than_classes;
  fewer_than_classes.length = 3;  // Four classes enabled.
  expect_invalid(fewer_than_classes, out);

  PasswordOptions fits;
  fits.length = 10;
  expect_invalid(fits, std::span(out).first(10));  // No room for the NUL.
}

TEST(PasswordGeneratorTest, CInterface) {
  CorePasswordOptions options{.length = 16,
                              .numbers = true,
                              .upper = false,
                              .lower = true,
                              .symbols = false,
                              .exclude_ambiguous = true};
  std::array<char, 17> out{};
  ASSERT_EQ(CoreGeneratePassword(&options, out.data(), out.size()), kCoreOk);
  EXPECT_EQ(std::strlen(out.data()), 16u);

  EXPECT_EQ(CoreGeneratePassword(&options, out.data(), 16),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreGeneratePassword(nullptr, out.data(), out.size()),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreGeneratePassword(&options, nullptr, out.size()),
            kCoreInvalidArgument);
}
