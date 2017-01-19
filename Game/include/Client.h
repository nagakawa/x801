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
#include <unordered_map>
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <SecureHandshake.h>
#include "packet.h"

namespace x801 {
  namespace game {
    class Client {
    public:
      Client(
          std::string ipAddress,
          uint16_t port
      ) : port(port),
          ipAddress(ipAddress) {
        initialise();
      }
      ~Client();
      Client(const Client& s) = delete;
      void operator=(const Client& s) = delete;
      const uint16_t port;
      std::string getIPAddress() const { return ipAddress; }
    private:
      void initialise();
      void handlePacket(
        uint8_t packetType,
        uint8_t* body, size_t length,
        RakNet::Packet* p
      );
      void handleLPacket(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Packet* p
      );
      void listen();
      RakNet::RakPeerInterface* peer = nullptr;
      std::string ipAddress;
      std::unordered_multimap<uint8_t, PacketCallback> callbacks;
      std::unordered_multimap<uint16_t, LPacketCallback> lCallbacks;
      // char* publicKey;
    };
  }
}