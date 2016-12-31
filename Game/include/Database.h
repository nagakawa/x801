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
#include <sqlite3.h>
#include "Credentials.h"

namespace x801 {
  namespace game {
    extern const char* DB_DIR;
    extern const char* DB_MAIN_PATH;
    extern const char* DB_AUTH_PATH;
    int stepBlock(sqlite3_stmt* statement, sqlite3* conn);
    // Note: this is not the database used for the autopatcher.
    class Database {
    public:
      Database();
      ~Database();
      Database(const Database& other) = delete;
      void operator=(const Database& other) = delete;
      void createAuthTable();
      void createUser(const char* username, const uint8_t* hash);
      void createUser(Credentials& c);
      void createUserDebug(std::string username, std::string password);
      // return true if succeeded
      bool getUserByID(uint32_t id, StoredCredentials& sc);
      bool getUserByName(const char* username, StoredCredentials& sc);
      // 0 if none
      uint32_t getUserIDByName(const char* username);
    private:
      sqlite3* me;
      sqlite3* auth;
      void open(sqlite3*& handle, const char* path);
      void userRowToSC(sqlite3_stmt* statement, StoredCredentials& sc);
    };
  }
}