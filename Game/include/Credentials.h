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
#include <string>

namespace x801 {
  namespace game {
    const int RAW_HASH_LENGTH = 256;
    const int COOKED_HASH_LENGTH = 20; // SHA-1 digest is 160 bits long
    const int SALT_LENGTH = 16;
    class Credentials {
    public:
      Credentials(std::string username, std::string password);
      ~Credentials();
      std::string getUsernameS() { return username; }
      const char* getUsername() { return username.c_str(); }
      // NOTE! spoils from getHash are invalidated when object is deleted
      uint8_t* getHash() { return hash; }
    private:
      std::string username;
      uint8_t* hash;
    };
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
      void operator=(const StoredCredentials& sc);
      ~StoredCredentials();
      uint32_t getUserID() { return userID; }
      std::string getUsernameS() { return username; }
      const char* getUsername() { return username.c_str(); }
      const uint8_t* getCookedHash() { return cookedHash; }
      const uint8_t* getSalt() { return salt; }
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
    };
  }
}
