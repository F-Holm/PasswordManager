/*
 * Compiled as C (see tests/core/CMakeLists.txt) to prove that
 * core/core_interface.h is a valid C header, not just C++ with extern "C".
 */
#include "core/core_interface.h"

#include <string.h>

int CoreCHeaderSmokeTest(void);

int CoreCHeaderSmokeTest(void) {
  /* Sizes are compile-time constants usable for C array bounds. */
  char description[kCoreDescriptionSize];
  CoreAccount account;
  CoreAccountEntry entry;
  uint8_t key[kCoreMasterKeySize];

  memset(description, 0, sizeof description);
  memset(&account, 0, sizeof account);
  memset(&entry, 0, sizeof entry);
  memset(key, 0, sizeof key);

  if (sizeof account.description != sizeof description) {
    return 1;
  }
  /* Null arguments must fail safely from C as well. */
  if (CoreOpenWithKey(NULL, "vault", key, sizeof key) !=
      kCoreInvalidArgument) {
    return 2;
  }
  if (CoreIsOpen(NULL)) {
    return 3;
  }
  if (CoreAddAccount(NULL, &account) != kCoreInvalidArgument) {
    return 4;
  }
  if (CoreResultName(kCoreOk) == NULL) {
    return 5;
  }
  CoreClose(NULL);
  CoreFree(NULL);
  return 0;
}
