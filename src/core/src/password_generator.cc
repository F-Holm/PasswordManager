#include "core/password_generator.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "crypto/crypto.h"
#include "types/account_size.h"

static_assert(kMaxGeneratedPasswordLength == AccountSize::kPassword - 1);

namespace {

constexpr std::string_view kNumbers = "0123456789";
constexpr std::string_view kUpper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr std::string_view kLower = "abcdefghijklmnopqrstuvwxyz";
constexpr std::string_view kSymbols = R"(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)";
constexpr std::string_view kAmbiguous = R"(0Oo1lI|`'")";

static_assert(kSymbols.size() == 32);

std::string Filter(std::string_view chars, bool exclude_ambiguous) {
  std::string result;
  for (const char c : chars) {
    if (!exclude_ambiguous || kAmbiguous.find(c) == std::string_view::npos) {
      result.push_back(c);
    }
  }
  return result;
}

// Returns a uniformly distributed integer in [0, bound) using rejection
// sampling, so that no value is more likely than another (a plain `% bound`
// would favour the low values whenever bound does not divide 2^32).
bool UniformBelow(std::uint32_t bound, std::uint32_t& out) {
  // Largest multiple of `bound` that fits in 2^32; values at or above it
  // are rejected.
  const std::uint64_t limit =
      (std::uint64_t{1} << 32U) - ((std::uint64_t{1} << 32U) % bound);
  while (true) {
    std::array<std::byte, sizeof(std::uint32_t)> bytes{};
    if (!Crypto::RandomBytes(bytes)) {
      return false;
    }
    std::uint32_t value = 0;
    std::memcpy(&value, bytes.data(), sizeof(value));
    if (value < limit) {
      out = value % bound;
      return true;
    }
  }
}

bool PickFrom(std::string_view alphabet, char& out) {
  std::uint32_t index = 0;
  if (!UniformBelow(static_cast<std::uint32_t>(alphabet.size()), index)) {
    return false;
  }
  out = alphabet[index];
  return true;
}

}  // namespace

CoreStatus GeneratePassword(const PasswordOptions& options,
                            std::span<char> out) {
  std::ranges::fill(out, '\0');

  std::vector<std::string> classes;
  const std::pair<bool, std::string_view> requested[] = {
      {options.numbers, kNumbers},
      {options.upper, kUpper},
      {options.lower, kLower},
      {options.symbols, kSymbols}};
  for (const auto& [enabled, chars] : requested) {
    if (enabled) {
      classes.push_back(Filter(chars, options.exclude_ambiguous));
    }
  }

  const std::size_t length = options.length;
  if (classes.empty() || length == 0 || length > kMaxGeneratedPasswordLength ||
      length < classes.size() || out.size() < length + 1) {
    return CoreStatus::kInvalidArgument;
  }

  std::string alphabet;
  for (const std::string& chars : classes) {
    alphabet += chars;
  }

  // One character from each enabled class, then the rest from the whole
  // alphabet, then a Fisher-Yates shuffle so the guaranteed characters are
  // not always at the front.
  bool ok = true;
  for (std::size_t i = 0; ok && i < classes.size(); ++i) {
    ok = PickFrom(classes[i], out[i]);
  }
  for (std::size_t i = classes.size(); ok && i < length; ++i) {
    ok = PickFrom(alphabet, out[i]);
  }
  for (std::size_t i = length - 1; ok && i > 0; --i) {
    std::uint32_t j = 0;
    ok = UniformBelow(static_cast<std::uint32_t>(i + 1), j);
    if (ok) {
      std::swap(out[i], out[j]);
    }
  }

  if (!ok) {
    Crypto::SecureClear(std::as_writable_bytes(out));
    return CoreStatus::kError;
  }
  out[length] = '\0';
  return CoreStatus::kOk;
}
