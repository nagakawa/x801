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
    enum ClientOrServer { HUH, CLIENT, SERVER, DB_ADD_USER };
    struct CLineConfig {
      ClientOrServer mode = HUH;
      std::string ip = "localhost";
      std::string username = "";
      std::string password = "";
      uint16_t port = 0;
      bool useIPV6 = false;
      bool debug = false;
    };
  }
}
