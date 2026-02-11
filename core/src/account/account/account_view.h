#pragma once

#include <cstddef>
#include <span>

struct AccountView {
  std::span<std::byte> description;
  std::span<std::byte> email;
  std::span<std::byte> user_name;
  std::span<std::byte> password;
  std::span<std::byte> extra;

  void Clear() noexcept;
  ~AccountView() noexcept;
};
