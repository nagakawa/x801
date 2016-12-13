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
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <SecureHandshake.h>

namespace x801 {
  namespace game {
    const int DEFAULT_MAX_CONNECTIONS = 1024;
    extern const char* KEY_DIR;
    extern const char* KEY_PUBLIC;
    extern const char* KEY_PRIVATE;
    class Server {
      Server(
          uint16_t port,
          unsigned short maxConnections = DEFAULT_MAX_CONNECTIONS
      ) : maxConnections(maxConnections), port(port) {
        initialise();
      }
      ~Server();
      Server(const Server& s) = delete;
      void operator=(const Server& s) = delete;
      const unsigned short maxConnections;
      const uint16_t port;
    private:
      void initialise();
      void updateKeyFiles();
      RakNet::RakPeerInterface* peer = nullptr;
      char* publicKey = nullptr;
      char* privateKey = nullptr;
    };
  }
}
