#include "gestor_contra.hh"

#include "set_rand.hh"

#include <iostream>

auto run() -> int {
  SetRand();
  std::cout << "Funciona" << std::endl;
  return 0;
}