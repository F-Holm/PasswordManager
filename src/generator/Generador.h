#pragma once

#include <string>

namespace Generador {
enum class TipoContrasenia : unsigned short {
  COMPLETA = 0,
  ALFANUMERICA = 1,
  ALFANUMERICA_MAYUSCULA = 2,
  ALFANUMERICA_MINUSCULA = 3,
  ALFABETICA = 4,
  ALFABETICA_MAYUSCULA = 5,
  ALFABETICA_MINUSCULA = 6,
  NUMERICA = 7
};
std::string generarContrasenia(const size_t CANT_CARACTERES,
                               const TipoContrasenia &TIPO);
} // namespace Generador