#include "Generador.h"

#include <random>

using std::string;

namespace {
void SetRand() {
  std::random_device rd;
  srand(static_cast<unsigned int>(std::time(nullptr)) ^ rd());
}

auto CaracterRandom(const char &base, const uint8_t &rango) -> char {
  return static_cast<char>((rand() % rango) + base);
}

auto ContraFacil(const string &contra, const Gen::TipoContra &tipo) -> bool {
  bool min = false, may = false, num = false, sim = false;
  for (char i : contra) {
    if (islower(i) != 0)
      min = true;
    else if (isupper(i) != 0)
      may = true;
    else if (isdigit(i) != 0)
      num = true;
    else
      sim = true;
  }

  switch (tipo) {
  case Gen::TipoContra::COMP:
    return !(min && may && num && sim);
    break;
  case Gen::TipoContra::ALFA_NUM:
    return !(min && may && num);
    break;
  case Gen::TipoContra::ALFA_NUM_MAY:
    return !(may && num);
    break;
  case Gen::TipoContra::ALFA_NUM_MIN:
    return !(min && num);
    break;
  case Gen::TipoContra::ALFA:
    return !(min && may);
    break;
  case Gen::TipoContra::ALFA_MAY:
  case Gen::TipoContra::ALFA_MIN:
  case Gen::TipoContra::NUM:
  default:
    return false;
    break;
  }
}

auto Caracter(const Gen::TipoContra &tipo) -> char {
  char cars[3] = {'X', 'X', 'X'};
  switch (tipo) {
  case Gen::TipoContra::COMP:
    return CaracterRandom('!', 94);
    break;
  case Gen::TipoContra::ALFA_NUM:
    cars[0] = CaracterRandom('a', 26);
    cars[1] = CaracterRandom('A', 26);
    cars[2] = CaracterRandom('0', 10);
    return cars[rand() % 3];
    break;
  case Gen::TipoContra::ALFA_NUM_MAY:
    cars[0] = CaracterRandom('A', 26);
    cars[1] = CaracterRandom('0', 10);
    return cars[rand() % 2];
    break;
  case Gen::TipoContra::ALFA_NUM_MIN:
    cars[0] = CaracterRandom('a', 26);
    cars[1] = CaracterRandom('0', 10);
    return cars[rand() % 2];
    break;
  case Gen::TipoContra::ALFA:
    cars[0] = CaracterRandom('a', 26);
    cars[1] = CaracterRandom('A', 26);
    return cars[rand() % 2];
    break;
  case Gen::TipoContra::ALFA_MAY:
    return CaracterRandom('A', 26);
    break;
  case Gen::TipoContra::ALFA_MIN:
    return CaracterRandom('a', 26);
    break;
  case Gen::TipoContra::NUM:
    return CaracterRandom('0', 10);
    break;
  default:
    return 'X';
    break;
  }
}
}  // namespace

auto Gen::GenerarContra(const size_t &cant_car, const Gen::TipoContra &tipo) -> string {
  SetRand();
  string contra = "";
  do {
    for (size_t i = 0; i < cant_car; i++)
      contra += Caracter(tipo);
  } while (ContraFacil(contra, tipo));
  return contra;
}