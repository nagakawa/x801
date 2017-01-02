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
#include <functional>
#include <MessageIdentifiers.h>
#include <RakNetTypes.h>

namespace x801 {
  namespace game {
    enum PacketIDs {
      PACKET_MOTD = 175,
      PACKET_LOGIN,
      PACKET_IM_LOGGED_IN,
      PACKET_UNRECOGNISED_COOKIE,
    };
    enum LoggedPacketIDs {
      LPACKET_CHAT = 0,
      LPACKET_RECEIVED_CHAT,
      LPACKET_MOVE,
    };
    static_assert((RakNet::MessageID) PACKET_MOTD > ID_USER_PACKET_ENUM,
        "RakNet defined too many pre-defined packet types!");
    uint8_t getPacketType(RakNet::Packet* p);
    size_t getPacketOffset(RakNet::Packet* p);
    // To use this implement the following methods for T:
    // void handlePacket(uint8_t packetType, uint8_t* body, size_t length, RakNet::Packet* p);
    // void handleLPacket(uint16_t lPacketType, uint8_t* cookie, uint8_t* lbody, size_t llength, RakNet::Packet* p);
    template <typename T>
    void listenToPackets(T& t) {
      while (true) {
        for (
            RakNet::Packet* p = t.peer->Receive();
            p != nullptr;
            t.peer->DeallocatePacket(p), p = t.peer->Receive()) {
          uint8_t packetType = getPacketType(p);
          size_t offset = getPacketOffset(p);
          uint8_t* body = p->data + offset;
          size_t length = p->length - offset;
          if (packetType == PACKET_IM_LOGGED_IN) {
            uint16_t lpacketType = (body[0] << 8) | body[1];
            uint8_t* cookie = body + 2;
            uint8_t* lbody = body + 10;
            size_t llength = length - 10;
            t.handleLPacket(lpacketType, cookie, lbody, llength, p);
          } else {
            t.handlePacket(packetType, body, length, p);
          }
        }
      }
    }
  }
}