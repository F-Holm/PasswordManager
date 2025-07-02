#include "Generador.h"

#include <random>

using std::string;

static void setRand() {
  std::random_device rd;
  srand(static_cast<unsigned int>(std::time(nullptr)) ^ rd());
}

static unsigned char caracterRandom(const unsigned char &BASE,
                                    const uint8_t RANGO) {
  return (rand() % RANGO) + BASE;
}

static bool contraFacil(string contra, const Generador::TipoContrasenia &TIPO) {
  bool minuscula = false, mayuscula = false, numero = false, simbolo = false;
  for (size_t i = 0; i < contra.size(); i++) {
    if (islower(contra[i]))
      minuscula = true;
    else if (isupper(contra[i]))
      mayuscula = true;
    else if (isdigit(contra[i]))
      numero = true;
    else
      simbolo = true;
  }

  switch (TIPO) {
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

static unsigned char caracter(const Generador::TipoContrasenia &TIPO) {
  char caracteres[3] = {'X', 'X', 'X'};
  switch (TIPO) {
  case Generador::TipoContrasenia::COMPLETA:
    return caracterRandom('!', 94);
    break;
  case Generador::TipoContrasenia::ALFANUMERICA:
    caracteres[0] = caracterRandom('a', 26);
    caracteres[1] = caracterRandom('A', 26);
    caracteres[2] = caracterRandom('0', 10);
    return caracteres[rand() % 3];
    break;
  case Generador::TipoContrasenia::ALFANUMERICA_MAYUSCULA:
    caracteres[0] = caracterRandom('A', 26);
    caracteres[1] = caracterRandom('0', 10);
    return caracteres[rand() % 2];
    break;
  case Generador::TipoContrasenia::ALFANUMERICA_MINUSCULA:
    caracteres[0] = caracterRandom('a', 26);
    caracteres[1] = caracterRandom('0', 10);
    return caracteres[rand() % 2];
    break;
  case Generador::TipoContrasenia::ALFABETICA:
    caracteres[0] = caracterRandom('a', 26);
    caracteres[1] = caracterRandom('A', 26);
    return caracteres[rand() % 2];
    break;
  case Generador::TipoContrasenia::ALFABETICA_MAYUSCULA:
    return caracterRandom('A', 26);
    break;
  case Generador::TipoContrasenia::ALFABETICA_MINUSCULA:
    return caracterRandom('a', 26);
    break;
  case Generador::TipoContrasenia::NUMERICA:
    return caracterRandom('0', 10);
    break;
  default:
    return 'X';
    break;
  }
}

string Generador::generarContrasenia(const size_t CANT_CARACTERES,
                                     const Generador::TipoContrasenia &TIPO) {
  setRand();
  string contra = "";
  do {
    for (size_t i = 0; i < CANT_CARACTERES; i++)
      contra += caracter(TIPO);
  } while (contraFacil(contra, TIPO));
  return contra;
}