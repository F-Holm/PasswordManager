#include "core/core_interface.h"

#include <array>
#include <cstring>
#include <new>

#include "core/core.h"
#include "crypto/crypto.h"
#include "types/account_size.h"

static_assert(kCoreDescriptionSize == AccountSize::kDescription);
static_assert(kCoreEmailSize == AccountSize::kEmail);
static_assert(kCoreUsernameSize == AccountSize::kUsername);
static_assert(kCorePasswordSize == AccountSize::kPassword);
static_assert(kCoreExtraSize == AccountSize::kExtra);
static_assert(kCoreMasterKeySize == Crypto::Size::kKey);
static_assert(sizeof(CoreAccount) ==
              AccountSize::kDescription + AccountSize::kEmail +
                  AccountSize::kUsername + AccountSize::kPassword +
                  AccountSize::kExtra);

// CoreSession simply wraps a Core; it exists so the public header can expose
// an opaque pointer instead of the C++ class definition.
struct CoreSession {
  Core core;
};

namespace {

void ToPlain(const CoreAccount& in, AccountPlain& out) noexcept {
  std::memcpy(out.description.data(), in.description, out.description.size());
  std::memcpy(out.email.data(), in.email, out.email.size());
  std::memcpy(out.username.data(), in.username, out.username.size());
  std::memcpy(out.password.data(), in.password, out.password.size());
  std::memcpy(out.extra.data(), in.extra, out.extra.size());
}

void FromPlain(const AccountPlain& in, CoreAccount& out) noexcept {
  std::memcpy(out.description, in.description.data(), in.description.size());
  std::memcpy(out.email, in.email.data(), in.email.size());
  std::memcpy(out.username, in.username.data(), in.username.size());
  std::memcpy(out.password, in.password.data(), in.password.size());
  std::memcpy(out.extra, in.extra.data(), in.extra.size());
}

}  // namespace

extern "C" {

CoreSession* CoreOpen(const char* db_path, const std::uint8_t* master_key,
                      std::size_t master_key_size) {
  if (db_path == nullptr || master_key == nullptr ||
      master_key_size != kCoreMasterKeySize) {
    return nullptr;
  }

  try {
    std::array<std::byte, Crypto::Size::kKey> key{};
    std::memcpy(key.data(), master_key, key.size());

    auto* session = new CoreSession();
    const bool opened = session->core.Open(db_path, key);
    Crypto::SecureClear(std::span(key));

    if (!opened) {
      delete session;
      return nullptr;
    }
    return session;
  } catch (...) {
    return nullptr;
  }
}

void CoreClose(CoreSession* session) {
  delete session;
}

bool CoreIsOpen(const CoreSession* session) {
  if (session == nullptr) {
    return false;
  }
  try {
    return session->core.IsOpen();
  } catch (...) {
    return false;
  }
}

bool CoreAddAccount(CoreSession* session, const CoreAccount* account) {
  if (session == nullptr || account == nullptr) {
    return false;
  }
  try {
    AccountPlain plain;
    ToPlain(*account, plain);
    return session->core.AddAccount(plain);
  } catch (...) {
    return false;
  }
}

bool CoreEditAccount(CoreSession* session, std::int64_t position,
                     const CoreAccount* account) {
  if (session == nullptr || account == nullptr) {
    return false;
  }
  try {
    AccountPlain plain;
    ToPlain(*account, plain);
    return session->core.EditAccount(position, plain);
  } catch (...) {
    return false;
  }
}

bool CoreRemoveAccount(CoreSession* session, std::int64_t position) {
  if (session == nullptr) {
    return false;
  }
  try {
    return session->core.RemoveAccount(position);
  } catch (...) {
    return false;
  }
}

bool CoreReorderAccount(CoreSession* session, std::int64_t from_position,
                        std::int64_t to_position) {
  if (session == nullptr) {
    return false;
  }
  try {
    return session->core.ReorderAccount(from_position, to_position);
  } catch (...) {
    return false;
  }
}

std::size_t CoreLoadIndex(CoreSession* session) {
  if (session == nullptr) {
    return 0;
  }
  try {
    return session->core.GetAllIndex().size();
  } catch (...) {
    return 0;
  }
}

bool CoreGetIndexEntry(const CoreSession* session, std::size_t list_index,
                       CoreAccountEntry* out_entry) {
  if (session == nullptr || out_entry == nullptr) {
    return false;
  }
  try {
    const std::vector<AccountEntry>& index = session->core.IndexCache();
    if (list_index >= index.size()) {
      return false;
    }
    const AccountEntry& entry = index[list_index];
    out_entry->position = entry.position;
    std::memcpy(out_entry->description, entry.description.data(),
                entry.description.size());
    return true;
  } catch (...) {
    return false;
  }
}

void CoreClearIndex(CoreSession* session) {
  if (session == nullptr) {
    return;
  }
  session->core.ClearIndex();
}

bool CoreGetAccount(CoreSession* session, std::int64_t position,
                    CoreAccount* out_account) {
  if (session == nullptr || out_account == nullptr) {
    return false;
  }
  try {
    const AccountPlain* account = session->core.GetAccount(position);
    if (account == nullptr) {
      return false;
    }
    FromPlain(*account, *out_account);
    return true;
  } catch (...) {
    return false;
  }
}

void CoreClearAccount(CoreSession* session) {
  if (session == nullptr) {
    return;
  }
  session->core.ClearAccount();
}

}  // extern "C"
