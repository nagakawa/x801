#include "Credentials.h"

/*
Copyright (C) 2016 AGC.


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.


You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

using namespace x801::game;

#include <string.h>
#include <argon2.h>

// Don't change the salt, or else logins will stop working.
static const char SALT[] = "This is not the real salt";
static const int SALT_LENGTH = sizeof(SALT) / sizeof(*SALT) - 1;
static const int TIME_COST = 40;
static const int MEMORY_COST = 12;
static const int PARALLELISM = 1;

x801::game::Credentials::Credentials(std::string username, std::string password) {
  this->username = username;
  hash = new uint8_t[RAW_HASH_LENGTH];
  argon2i_hash_raw(
    TIME_COST, MEMORY_COST, PARALLELISM,
    password.c_str(), password.length(),
    SALT, SALT_LENGTH,
    hash, RAW_HASH_LENGTH
  );
}

x801::game::Credentials::~Credentials() {
  delete[] hash;
}

x801::game::StoredCredentials::StoredCredentials(
    uint32_t userID,
    std::string username,
    const uint8_t* cookedHash,
    const uint8_t* salt) {
  build(userID, username, cookedHash, salt);
}

x801::game::StoredCredentials::StoredCredentials(const StoredCredentials& sc) {
  build(sc.userID, sc.username, sc.cookedHash, sc.salt);
}

x801::game::StoredCredentials::StoredCredentials(StoredCredentials&& sc) {
  build(
    std::move(sc.userID), std::move(sc.username),
    std::move(sc.cookedHash), std::move(sc.salt));
}

void x801::game::StoredCredentials::operator=(const StoredCredentials& sc) {
  delete[] cookedHash;
  delete[] salt;
  build(sc.userID, sc.username, sc.cookedHash, sc.salt);
}

void x801::game::StoredCredentials::build(
    uint32_t userID,
    std::string username,
    const uint8_t* cookedHash,
    const uint8_t* salt) {
  this->userID = userID;
  this->username = username;
  this->cookedHash = new uint8_t[COOKED_HASH_LENGTH];
  this->salt = new uint8_t[SALT_LENGTH];
  memcpy(this->cookedHash, cookedHash, COOKED_HASH_LENGTH);
  memcpy(this->salt, salt, SALT_LENGTH);
}

x801::game::StoredCredentials::~StoredCredentials() {
  delete[] cookedHash;
  delete[] salt;
}