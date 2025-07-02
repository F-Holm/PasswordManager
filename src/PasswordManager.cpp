// PasswordManager.cpp : Defines the entry point for the application.
//

#include "PasswordManager.h"
#include <openssl/sha.h>

using namespace std;

int main() {
  cout << "Hello CMake. -> " << time(0) % 1000 << " -> " << SHA256_DIGEST_LENGTH
       << endl;
  // system("pause");
  return 0;
}