#pragma once

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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error Only C++11 or later supported.
#endif

#include <stdint.h>
#include <string.h>
#include <string>

namespace x801 {
  namespace game {
    const int RAW_HASH_LENGTH = 256;
    const int COOKED_HASH_LENGTH = 20; // SHA-1 digest is 160 bits long
    const int SALT_LENGTH = 16;
    class StoredCredentials {
    public:
      StoredCredentials() :
          userID(0), username(""),
          cookedHash(nullptr), salt(nullptr) {}
      StoredCredentials(
        uint32_t userID,
        std::string username,
        const uint8_t* cookedHash,
        const uint8_t* salt);
      StoredCredentials(const StoredCredentials& sc);
      StoredCredentials(StoredCredentials&& sc);
      StoredCredentials& operator=(const StoredCredentials& sc);
      ~StoredCredentials();
      uint32_t getUserID() const { return userID; }
      std::string getUsernameS() const { return username; }
      const char* getUsername() const { return username.c_str(); }
      const uint8_t* getCookedHash() const { return cookedHash; }
      const uint8_t* getSalt() const { return salt; }
    private:
      uint32_t userID;
      std::string username;
      uint8_t* cookedHash;
      uint8_t* salt;
      void build(
        uint32_t userID,
        std::string username,
        const uint8_t* cookedHash,
        const uint8_t* salt);
      void replace(
        uint32_t userID,
        std::string username,
        const uint8_t* cookedHash,
        const uint8_t* salt);
    };
    class Credentials {
    public:
      Credentials() : username(""), hash(nullptr) {}
      Credentials(const std::string& username, const std::string& password) :
        username(username), hash(new uint8_t[RAW_HASH_LENGTH]) {
        fillHash(password);
      }
      Credentials(const std::string& username, const uint8_t* h) :
        username(username), hash(new uint8_t[RAW_HASH_LENGTH]) {
        memcpy(hash, h, RAW_HASH_LENGTH);
      }
      Credentials(const Credentials& other) :
        username(other.username), hash(new uint8_t[RAW_HASH_LENGTH]) {
        memcpy(hash, other.hash, RAW_HASH_LENGTH);
      }
      Credentials& operator=(const Credentials& other);
      ~Credentials();
      std::string getUsernameS() const { return username; }
      const char* getUsername() const { return username.c_str(); }
      // NOTE! spoils from getHash are invalidated when object is deleted
      uint8_t* getHash() const { return hash; }
      bool matches(StoredCredentials& sc) const;
    private:
      std::string username;
      uint8_t* hash;
      void fillHash(const std::string& password);
    };
  }
}
