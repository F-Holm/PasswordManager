#include "generador.hh"

namespace {
auto CaracterRandom(const char &base, const uint8_t &rango) -> char { return static_cast<char>((rand() % rango) + base); }

auto ContraFacil(const std::string &contra, const Generador::TipoContra &tipo) -> bool {
  if (contra.size() <= 4)
    return false;

  bool min = false, may = false, num = false, sim = false;
  for (char chr : contra) {
    if (islower(chr) != 0) {
      min = true;
    } else if (isupper(chr) != 0) {
      may = true;
    } else if (isdigit(chr) != 0) {
      num = true;
    } else {
      sim = true;
    }
  }

  switch (tipo) {
    case Generador::TipoContra::COMP:
      return !(min && may && num && sim);
      break;
    case Generador::TipoContra::ALFA_NUM:
      return !(min && may && num);
      break;
    case Generador::TipoContra::ALFA_NUM_MAY:
      return !(may && num);
      break;
    case Generador::TipoContra::ALFA_NUM_MIN:
      return !(min && num);
      break;
    case Generador::TipoContra::ALFA:
      return !(min && may);
      break;
    case Generador::TipoContra::ALFA_MAY:
    case Generador::TipoContra::ALFA_MIN:
    case Generador::TipoContra::NUM:
    default:
      return false;
      break;
  }
}

auto Caracter(const Generador::TipoContra &tipo) -> char {
  char cars[3] = {'X', 'X', 'X'};
  switch (tipo) {
    case Generador::TipoContra::COMP:
      return CaracterRandom('!', 94);
      break;
    case Generador::TipoContra::ALFA_NUM:
      cars[0] = CaracterRandom('a', 26);
      cars[1] = CaracterRandom('A', 26);
      cars[2] = CaracterRandom('0', 10);
      return cars[rand() % 3];
      break;
    case Generador::TipoContra::ALFA_NUM_MAY:
      cars[0] = CaracterRandom('A', 26);
      cars[1] = CaracterRandom('0', 10);
      return cars[rand() % 2];
      break;
    case Generador::TipoContra::ALFA_NUM_MIN:
      cars[0] = CaracterRandom('a', 26);
      cars[1] = CaracterRandom('0', 10);
      return cars[rand() % 2];
      break;
    case Generador::TipoContra::ALFA:
      cars[0] = CaracterRandom('a', 26);
      cars[1] = CaracterRandom('A', 26);
      return cars[rand() % 2];
      break;
    case Generador::TipoContra::ALFA_MAY:
      return CaracterRandom('A', 26);
      break;
    case Generador::TipoContra::ALFA_MIN:
      return CaracterRandom('a', 26);
      break;
    case Generador::TipoContra::NUM:
      return CaracterRandom('0', 10);
      break;
    default:
      return 'X';
      break;
  }
}
}  // namespace

auto Generador::GenerarContra(const size_t &largo, const Generador::TipoContra &tipo) -> std::string {
  std::string contra;
  do {
    contra = "";
    for (size_t i = 0; i < largo; i++) {
      contra += Caracter(tipo);
    }
  } while (ContraFacil(contra, tipo));
  return contra;
}