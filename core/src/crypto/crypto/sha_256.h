#pragma once

namespace Sha256 {
void Sha256::Sha256(std::string_view str, char* rta) noexcept;
void Sha256::Sha256_X(std::string_view str, char* rta, size_t len_rta) noexcept;
}  // namespace Sha256
