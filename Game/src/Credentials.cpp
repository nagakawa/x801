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

#include <argon2.h>
#include "sha1.h"

// Don't change the salt, or else logins will stop working.
static const char SALT[] = "This is not the real salt";
static const int SALT_LENGTH = sizeof(SALT) / sizeof(*SALT) - 1;
static const int TIME_COST = 40;
static const int MEMORY_COST = 12;
static const int PARALLELISM = 1;

void x801::game::Credentials::fillHash(const std::string& password) {
  argon2i_hash_raw(
    TIME_COST, MEMORY_COST, PARALLELISM,
    password.c_str(), password.length(),
    SALT, SALT_LENGTH,
    hash, RAW_HASH_LENGTH
  );
}

Credentials& x801::game::Credentials::operator=(const Credentials& other) {
  username = other.username;
  memcpy(hash, other.hash, RAW_HASH_LENGTH);
  return *this;
}

x801::game::Credentials::~Credentials() {
  delete[] hash;
}

bool x801::game::Credentials::matches(StoredCredentials& sc) const {
  // Generate SHA-1 hash
  SHA1_CTX sha1;
  sha1_init(&sha1);
  sha1_update(&sha1, hash, RAW_HASH_LENGTH);
  sha1_update(&sha1, sc.getSalt(), SALT_LENGTH);
  uint8_t cooked[COOKED_HASH_LENGTH];
  sha1_final(&sha1, cooked);
  // Below code is the same as
  // return memcmp(cooked, sc.getCookedHash(), COOKED_HASH_LENGTH) == 0;
  // but not subject to timing attacks
  uint8_t res = 0;
  for (int i = 0; i < COOKED_HASH_LENGTH; ++i) {
    res |= cooked[i] ^ sc.getCookedHash()[i];
  }
  return res == 0;
}

#pragma GCC diagnostic push                // we DO want an explicit ctor since the header
#pragma GCC diagnostic ignored "-Weffc++"  // would otherwise be hard to read
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

StoredCredentials& x801::game::StoredCredentials::operator=(const StoredCredentials& sc) {
  replace(sc.userID, sc.username, sc.cookedHash, sc.salt);
  return *this;
}

void x801::game::StoredCredentials::build(
    uint32_t userID,
    std::string username,
    const uint8_t* cookedHash,
    const uint8_t* salt) {
  this->cookedHash = new uint8_t[COOKED_HASH_LENGTH];
  this->salt = new uint8_t[SALT_LENGTH];
  replace(userID, username, cookedHash, salt);
}

void x801::game::StoredCredentials::replace(
    uint32_t userID,
    std::string username,
    const uint8_t* cookedHash,
    const uint8_t* salt) {
  if (this->cookedHash == nullptr)
    this->cookedHash = new uint8_t[COOKED_HASH_LENGTH];
  if (this->salt == nullptr)
    this->salt = new uint8_t[SALT_LENGTH];
  this->userID = userID;
  this->username = username;
  memcpy(this->cookedHash, cookedHash, COOKED_HASH_LENGTH);
  memcpy(this->salt, salt, SALT_LENGTH);
}

#pragma GCC diagnostic pop

x801::game::StoredCredentials::~StoredCredentials() {
  delete[] cookedHash;
  delete[] salt;
}