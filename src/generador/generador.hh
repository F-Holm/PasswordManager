#pragma once

#include <string>
#include <cstdint>

namespace Generador {
enum class TipoContra : uint8_t {
  COMP = 0,
  ALFA_NUM = 1,
  ALFA_NUM_MAY = 2,
  ALFA_NUM_MIN = 3,
  ALFA = 4,
  ALFA_MAY = 5,
  ALFA_MIN = 6,
  NUM = 7
};

auto GenerarContra(const size_t &largo, const TipoContra &tipo) -> std::string;
} // namespace Generador