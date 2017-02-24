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
#include <sqlite3.h>
#include "Client.h"

namespace x801 {
  namespace game {
    extern const char* PATCHER_DIR;
    class Patcher {
    public:
      Patcher(const Client& c);
      ~Patcher();
    private:
      sqlite3* conn;
      void open(sqlite3*& handle, const char* path);
      void createFileTable();
      uint32_t latestVersion = 0;
      void createFileEntry(
        const char* fname,
        uint32_t version,
        uint32_t contentLength,
        const uint8_t* contents);
      bool getFileEntry(
        const char* fname,
        uint32_t& version,
        uint32_t& contentLength,
        uint8_t*& contents);
    };
  }
}