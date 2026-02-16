#pragma once

#include <cstddef>

#include "account/account_size.h"

extern "C" {

const std::size_t kDescription = AccountSize::kDescription;
const std::size_t kEmail = AccountSize::kEmail;
const std::size_t kUsername = AccountSize::kUsername;
const std::size_t kPassword = AccountSize::kPassword;
const std::size_t kExtra = AccountSize::kExtra;
const std::size_t kData = AccountSize::kData;

struct AccountPointer {
  char* description;
  char* email;
  char* username;
  char* password;
  char* extra;
};

struct Account {
  char description;
  char email;
  char username;
  char password;
  char extra;
};

Account newAccount;
Account editAccount;

void List(char* password, std::size_t password_size, char** list,
          std::size_t list_length);
void Get(char* password, std::size_t password_size, std::size_t id,
         AccountPointer account);
void Add(char* password, std::size_t password_size, Account account);
void Edit(char* password, std::size_t password_size, AccountPointer account);
void Remove(char* password, std::size_t password_size, std::size_t id);
void Rename(char* password, std::size_t password_size, std::size_t id,
            char* new_description);
void Generate(char* password, std::size_t length, bool numbers, bool upper,
              bool lower, bool symbols);
void ClearAll();
void ClearSelected();
void Refresh();
}
