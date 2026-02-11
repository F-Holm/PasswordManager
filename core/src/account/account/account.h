#pragma once

#include <cstddef>

namespace AccountSize {
  static constexpr std::size_t kDescription = 128;
  static constexpr std::size_t kEmail = 128;
  static constexpr std::size_t kUsername = 128;
  static constexpr std::size_t kPassword = 128;
  static constexpr std::size_t kExtra = 1024;
} // AccountSize