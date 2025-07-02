#pragma once

#include <string>

namespace Generador {
enum class TipoContrasenia : uint8_t {
  COMPLETA = 0,
  ALFANUMERICA = 1,
  ALFANUMERICA_MAYUSCULA = 2,
  ALFANUMERICA_MINUSCULA = 3,
  ALFABETICA = 4,
  ALFABETICA_MAYUSCULA = 5,
  ALFABETICA_MINUSCULA = 6,
  NUMERICA = 7
};

auto GenerarContrasenia(const size_t &cant_car, const TipoContrasenia &tipo) -> std::string;
} // namespace Generador