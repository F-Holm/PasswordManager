#pragma once

#include <cstddef>

namespace AccountSize {
static constexpr std::size_t kDescription = 128;
static constexpr std::size_t kEmail = 256;
static constexpr std::size_t kUsername = 128;
static constexpr std::size_t kPassword = 256;
static constexpr std::size_t kExtra = 1024;
static constexpr std::size_t kData = kEmail + kUsername + kPassword + kExtra;
}  // namespace AccountSize
