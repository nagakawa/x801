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
#include <array>
#include <map>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <SecureHandshake.h>
#include <utils.h>
#include "GameState.h"
#include "packet.h"

namespace x801 {
  namespace game {
    const int DEFAULT_MAX_CONNECTIONS = 1024;
    const int COOKIE_LEN = 16;
    extern const char* KEY_DIR;
    extern const char* KEY_PUBLIC;
    extern const char* KEY_PRIVATE;
    class Server {
    public:
      Server(
          uint16_t port,
          unsigned short maxConnections = DEFAULT_MAX_CONNECTIONS,
          bool useIPV6 = false
      ) : maxConnections(maxConnections), port(port),
          useIPV6(useIPV6), playersByCookie() {
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
      void handlePacket(
        uint8_t packetType,
        uint8_t* body, size_t length,
        RakNet::Packet* p
      );
      void handleLPacket(
        uint16_t lPacketType, uint8_t* cookie,
        uint8_t* lbody, size_t llength,
        RakNet::Packet* p
      );
      void listen();
      void logout(uint32_t playerID);
      void logoutByPacket(
        uint8_t packetType,
        uint8_t* body, size_t length,
        RakNet::Packet* p
      );
      void sendMOTD(
        uint8_t packetType,
        uint8_t* body, size_t length,
        RakNet::Packet* p
      );
      RakNet::RakPeerInterface* peer = nullptr;
      char* publicKey = nullptr;
      char* privateKey = nullptr;
      bool useIPV6;
      // When the client disconnects without sending a proper logout message,
      // all we have will be their address.
      std::map<RakNet::SystemAddress, uint32_t> playersByAddress;
      std::unordered_map<
        std::array<uint8_t, COOKIE_LEN>, uint32_t,
        x801::base::STDArrayHash<uint8_t, COOKIE_LEN>
      > playersByCookie;
      std::unordered_map<
        uint32_t, std::array<uint8_t, COOKIE_LEN>
      > cookiesByPlayer;
      std::unordered_multimap<uint8_t, PacketCallback> callbacks;
      std::unordered_multimap<uint16_t, LPacketCallback> lCallbacks;
      GameState g;
    };
  }
}
