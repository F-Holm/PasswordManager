#include "core/core_interface.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <new>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "core/core.h"
#include "core/core_status.h"
#include "core/password_generator.h"
#include "crypto/crypto.h"
#include "types/account_size.h"

static_assert(kCoreDescriptionSize == AccountSize::kDescription);
static_assert(kCoreEmailSize == AccountSize::kEmail);
static_assert(kCoreUsernameSize == AccountSize::kUsername);
static_assert(kCorePasswordSize == AccountSize::kPassword);
static_assert(kCoreExtraSize == AccountSize::kExtra);
static_assert(kCoreMaxGeneratedPasswordLength == kMaxGeneratedPasswordLength);
static_assert(kCoreAnyRevision == Core::kAnyRevision);
static_assert(sizeof(CoreAccount) ==
              sizeof(std::int64_t) + AccountSize::kDescription +
                  AccountSize::kEmail + AccountSize::kUsername +
                  AccountSize::kPassword + AccountSize::kExtra);

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
  out.revision = in.revision;
  std::memcpy(out.description, in.description.data(), in.description.size());
  std::memcpy(out.email, in.email.data(), in.email.size());
  std::memcpy(out.username, in.username.data(), in.username.size());
  std::memcpy(out.password, in.password.data(), in.password.size());
  std::memcpy(out.extra, in.extra.data(), in.extra.size());
}

}  // namespace

namespace {

constexpr CoreResult ToResult(CoreStatus status) noexcept {
  return static_cast<CoreResult>(status);
}

// Runs `body` (which returns a CoreStatus), turning any C++ exception into
// kCoreError so it never unwinds across the C ABI.
template <typename Body>
CoreResult Guard(Body&& body) noexcept {
  try {
    return ToResult(std::forward<Body>(body)());
  } catch (...) {
    return kCoreError;
  }
}

std::optional<Crypto::KdfParams> ToKdfParams(
    const CoreKdfParams* params) noexcept {
  if (params == nullptr) {
    return std::nullopt;
  }
  return Crypto::KdfParams{.memory_kib = params->memory_kib,
                           .iterations = params->iterations,
                           .parallelism = params->parallelism};
}

}  // namespace

static_assert(ToResult(CoreStatus::kOk) == kCoreOk);
static_assert(ToResult(CoreStatus::kNotFound) == kCoreNotFound);
static_assert(ToResult(CoreStatus::kConflict) == kCoreConflict);
static_assert(ToResult(CoreStatus::kBusy) == kCoreBusy);
static_assert(ToResult(CoreStatus::kLocked) == kCoreLocked);
static_assert(ToResult(CoreStatus::kWrongPassword) == kCoreWrongPassword);
static_assert(ToResult(CoreStatus::kInvalidArgument) == kCoreInvalidArgument);
static_assert(ToResult(CoreStatus::kAlreadyExists) == kCoreAlreadyExists);
static_assert(ToResult(CoreStatus::kCorrupted) == kCoreCorrupted);
static_assert(ToResult(CoreStatus::kError) == kCoreError);

extern "C" {

CoreResult CoreGeneratePassword(const CorePasswordOptions* options, char* out,
                                std::size_t out_size) {
  if (options == nullptr || out == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    const PasswordOptions cpp_options{
        .length = options->length,
        .numbers = options->numbers,
        .upper = options->upper,
        .lower = options->lower,
        .symbols = options->symbols,
        .exclude_ambiguous = options->exclude_ambiguous};
    return GeneratePassword(cpp_options, std::span(out, out_size));
  });
}

const char* CoreResultName(CoreResult result) {
  switch (result) {
    case kCoreOk:
    case kCoreNotFound:
    case kCoreConflict:
    case kCoreBusy:
    case kCoreLocked:
    case kCoreWrongPassword:
    case kCoreInvalidArgument:
    case kCoreAlreadyExists:
    case kCoreCorrupted:
    case kCoreError:
      // Every name is a string literal, hence NUL-terminated.
      return ToString(static_cast<CoreStatus>(result)).data();
    default:
      return "unknown";
  }
}

CoreSession* CoreNew(void) {
  return new (std::nothrow) CoreSession();
}

void CoreFree(CoreSession* session) {
  delete session;
}

CoreKdfParams CoreDefaultKdfParams(void) {
  return {.memory_kib = Crypto::kDefaultKdfParams.memory_kib,
          .iterations = Crypto::kDefaultKdfParams.iterations,
          .parallelism = Crypto::kDefaultKdfParams.parallelism};
}

CoreResult CoreCreate(CoreSession* session, const char* db_path,
                      const char* password, std::size_t password_size,
                      const CoreKdfParams* kdf_params) {
  if (session == nullptr || db_path == nullptr || password == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    return session->core.Create(
        db_path, std::string_view(password, password_size),
        ToKdfParams(kdf_params).value_or(Crypto::kDefaultKdfParams));
  });
}

CoreResult CoreUnlock(CoreSession* session, const char* db_path,
                      const char* password, std::size_t password_size) {
  if (session == nullptr || db_path == nullptr || password == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    return session->core.Unlock(db_path,
                                std::string_view(password, password_size));
  });
}

CoreResult CoreChangeMasterPassword(CoreSession* session,
                                    const char* current_password,
                                    std::size_t current_password_size,
                                    const char* new_password,
                                    std::size_t new_password_size,
                                    const CoreKdfParams* kdf_params) {
  if (session == nullptr || current_password == nullptr ||
      new_password == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    return session->core.ChangeMasterPassword(
        std::string_view(current_password, current_password_size),
        std::string_view(new_password, new_password_size),
        ToKdfParams(kdf_params));
  });
}

void CoreClose(CoreSession* session) {
  if (session != nullptr) {
    session->core.Close();
  }
}

bool CoreIsOpen(const CoreSession* session) {
  return session != nullptr && session->core.IsOpen();
}

CoreResult CoreAddAccount(CoreSession* session, const CoreAccount* account,
                          std::int64_t* out_id) {
  if (session == nullptr || account == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    AccountPlain plain;
    ToPlain(*account, plain);
    return session->core.AddAccount(plain, out_id);
  });
}

CoreResult CoreEditAccount(CoreSession* session, std::int64_t id,
                           std::int64_t expected_revision,
                           const CoreAccount* account) {
  if (session == nullptr || account == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    AccountPlain plain;
    ToPlain(*account, plain);
    return session->core.EditAccount(id, expected_revision, plain);
  });
}

CoreResult CoreRemoveAccount(CoreSession* session, std::int64_t id) {
  if (session == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] { return session->core.RemoveAccount(id); });
}

CoreResult CoreReorderAccount(CoreSession* session, std::int64_t id,
                              std::int64_t to_position) {
  if (session == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] { return session->core.ReorderAccount(id, to_position); });
}

CoreResult CoreLoadIndex(CoreSession* session, std::size_t* out_count) {
  if (session == nullptr || out_count == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    const CoreStatus status = session->core.LoadIndex();
    if (status == CoreStatus::kOk) {
      *out_count = session->core.Index().size();
    }
    return status;
  });
}

CoreResult CoreGetIndexEntry(const CoreSession* session, std::size_t list_index,
                             CoreAccountEntry* out_entry) {
  if (session == nullptr || out_entry == nullptr) {
    return kCoreInvalidArgument;
  }
  const std::vector<AccountEntry>& index = session->core.Index();
  if (list_index >= index.size()) {
    return kCoreNotFound;
  }
  const AccountEntry& entry = index[list_index];
  out_entry->id = entry.id;
  out_entry->position = entry.position;
  out_entry->revision = entry.revision;
  std::memcpy(out_entry->description, entry.description.data(),
              entry.description.size());
  return kCoreOk;
}

void CoreClearIndex(CoreSession* session) {
  if (session != nullptr) {
    session->core.ClearIndex();
  }
}

CoreResult CoreSearchIndex(const CoreSession* session, const char* query,
                           std::size_t query_size, std::size_t* out_indices,
                           std::size_t capacity, std::size_t* out_count) {
  if (session == nullptr || out_count == nullptr ||
      (query == nullptr && query_size > 0) ||
      (out_indices == nullptr && capacity > 0)) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    const std::vector<std::size_t> matches = session->core.SearchIndex(
        query == nullptr ? std::string_view{}
                         : std::string_view(query, query_size));
    const std::size_t copied = std::min(matches.size(), capacity);
    std::copy_n(matches.begin(), copied, out_indices);
    *out_count = matches.size();
    return CoreStatus::kOk;
  });
}

CoreResult CoreCheckExternalChanges(CoreSession* session, bool* out_changed) {
  if (session == nullptr || out_changed == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    const auto changed = session->core.CheckExternalChanges();
    if (!changed.has_value()) {
      return changed.error();
    }
    *out_changed = *changed;
    return CoreStatus::kOk;
  });
}

CoreResult CoreCount(CoreSession* session, std::size_t* out_count) {
  if (session == nullptr || out_count == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    const auto count = session->core.Count();
    if (!count.has_value()) {
      return count.error();
    }
    *out_count = *count;
    return CoreStatus::kOk;
  });
}

CoreResult CoreCompact(CoreSession* session) {
  if (session == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] { return session->core.Compact(); });
}

CoreResult CoreGetAccount(CoreSession* session, std::int64_t id,
                          CoreAccount* out_account) {
  if (session == nullptr || out_account == nullptr) {
    return kCoreInvalidArgument;
  }
  return Guard([&] {
    const auto account = session->core.GetAccount(id);
    if (!account.has_value()) {
      return account.error();
    }
    FromPlain(**account, *out_account);
    return CoreStatus::kOk;
  });
}

void CoreClearAccount(CoreSession* session) {
  if (session != nullptr) {
    session->core.ClearAccount();
  }
}

}  // extern "C"
