#include "set_rand.hh"

void SetRand() {
  std::random_device rd;
  srand(static_cast<unsigned int>(std::time(nullptr)) ^ rd());
}