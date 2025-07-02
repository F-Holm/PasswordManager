#include "Generador.h"

#include <random>

using std::string;

static void SetRand() {
  std::random_device rd;
  srand(static_cast<unsigned int>(std::time(nullptr)) ^ rd());
}

static auto CaracterRandom(const unsigned char &base,
                                    const uint8_t rango) -> char {
  return (rand() % rango) + base;
}

static auto ContraFacil(const string &contra, const Generador::TipoContrasenia &tipo) -> bool {
  bool minuscula = false, mayuscula = false, numero = false, simbolo = false;
  for (char i : contra) {
    if (islower(i))
      minuscula = true;
    else if (isupper(i))
      mayuscula = true;
    else if (isdigit(i))
      numero = true;
    else
      simbolo = true;
  }

  switch (tipo) {
  case Generador::TipoContrasenia::COMPLETA:
    return !(minuscula && mayuscula && numero && simbolo);
    break;
  case Generador::TipoContrasenia::ALFANUMERICA:
    return !(minuscula && mayuscula && numero);
    break;
  case Generador::TipoContrasenia::ALFANUMERICA_MAYUSCULA:
    return !(mayuscula && numero);
    break;
  case Generador::TipoContrasenia::ALFANUMERICA_MINUSCULA:
    return !(minuscula && numero);
    break;
  case Generador::TipoContrasenia::ALFABETICA:
    return !(minuscula && mayuscula);
    break;
  case Generador::TipoContrasenia::ALFABETICA_MAYUSCULA:
  case Generador::TipoContrasenia::ALFABETICA_MINUSCULA:
  case Generador::TipoContrasenia::NUMERICA:
  default:
    return false;
    break;
  }
}

static auto Caracter(const Generador::TipoContrasenia &tipo) -> char {
  char caracteres[3] = {'X', 'X', 'X'};
  switch (tipo) {
  case Generador::TipoContrasenia::COMPLETA:
    return CaracterRandom('!', 94);
    break;
  case Generador::TipoContrasenia::ALFANUMERICA:
    caracteres[0] = CaracterRandom('a', 26);
    caracteres[1] = CaracterRandom('A', 26);
    caracteres[2] = CaracterRandom('0', 10);
    return caracteres[rand() % 3];
    break;
  case Generador::TipoContrasenia::ALFANUMERICA_MAYUSCULA:
    caracteres[0] = CaracterRandom('A', 26);
    caracteres[1] = CaracterRandom('0', 10);
    return caracteres[rand() % 2];
    break;
  case Generador::TipoContrasenia::ALFANUMERICA_MINUSCULA:
    caracteres[0] = CaracterRandom('a', 26);
    caracteres[1] = CaracterRandom('0', 10);
    return caracteres[rand() % 2];
    break;
  case Generador::TipoContrasenia::ALFABETICA:
    caracteres[0] = CaracterRandom('a', 26);
    caracteres[1] = CaracterRandom('A', 26);
    return caracteres[rand() % 2];
    break;
  case Generador::TipoContrasenia::ALFABETICA_MAYUSCULA:
    return CaracterRandom('A', 26);
    break;
  case Generador::TipoContrasenia::ALFABETICA_MINUSCULA:
    return CaracterRandom('a', 26);
    break;
  case Generador::TipoContrasenia::NUMERICA:
    return CaracterRandom('0', 10);
    break;
  default:
    return 'X';
    break;
  }
}

auto Generador::GenerarContrasenia(const size_t &cant_car, const Generador::TipoContrasenia &tipo) -> string {
  SetRand();
  string contra = "";
  do {
    for (size_t i = 0; i < cant_car; i++)
      contra += Caracter(tipo);
  } while (ContraFacil(contra, tipo));
  return contra;
}