#include "utils/set_rand.h"

#include <cstdlib>
#include <ctime>
#include <random>

void SetRand() {
  std::random_device rd;
  srand(static_cast<unsigned int>(std::time(nullptr)) ^ rd());
}
