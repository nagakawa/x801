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
#include <BitStream.h>
#include <MessageIdentifiers.h>
#include <RakNetTypes.h>

#define MAKE_PACKET_CALLBACK(method, count) { \
  [this]( \
    uint8_t packetType, \
    uint8_t* body, size_t length, \
    RakNet::Time t, \
    RakNet::Packet* p) { \
      (void) t; \
      this->method(packetType, body, length, p); \
    }, count \
}
#define MAKE_LPACKET_CALLBACK(method, count) { \
  [this]( \
    uint16_t lpacketType, uint32_t userID, \
    uint8_t* lbody, size_t llength, \
    RakNet::Time t, \
    RakNet::Packet* p) { \
      (void) t; \
      this->method(lpacketType, userID, lbody, llength, p); \
    }, count \
}
#define MAKE_LPACKET_CALLBACK_CLIENT(method, count) { \
  [this]( \
    uint16_t lpacketType, uint32_t userID, \
    uint8_t* lbody, size_t llength, \
    RakNet::Time t, \
    RakNet::Packet* p) { \
      (void) t; (void) userID; \
      this->method(lpacketType, lbody, llength, p); \
    }, count \
}
#define MAKE_LPACKET_CALLBACK_TIMED(method, count) { \
  [this]( \
    uint16_t lpacketType, uint32_t userID, \
    uint8_t* lbody, size_t llength, \
    RakNet::Time t, \
    RakNet::Packet* p) { \
      this->method(lpacketType, userID, lbody, llength, t, p); \
    }, count \
}
#define MAKE_LPACKET_CALLBACK_CLIENT_TIMED(method, count) { \
  [this]( \
    uint16_t lpacketType, uint32_t userID, \
    uint8_t* lbody, size_t llength, \
    RakNet::Time t, \
    RakNet::Packet* p) { \
      (void) userID; \
      this->method(lpacketType, lbody, llength, t, p); \
    }, count \
}

namespace x801 {
  namespace game {
    enum PacketIDs {
      PACKET_MOTD = 175,
      PACKET_LOGIN,
      PACKET_IM_LOGGED_IN,
      PACKET_UNRECOGNISED_COOKIE,
      PACKET_FILE,
    };
    enum LoggedPacketIDs {
      LPACKET_CHAT = 0,
      LPACKET_RECEIVE_CHAT,
      LPACKET_MOVE,
      LPACKET_FILE,
      LPACKET_IDENTIFY,
      LPACKET_CHANGE_AREA,
    };
    enum ChatStatus {
      CHAT_OK = 0,
      CHAT_NO_PERMISSION,
      CHAT_MUTED,
    };
    static_assert((RakNet::MessageID) PACKET_MOTD > ID_USER_PACKET_ENUM,
        "RakNet defined too many pre-defined packet types!");
    uint8_t getPacketType(RakNet::Packet* p);
    size_t getPacketOffset(RakNet::Packet* p);
    void writeStringToBitstream32(RakNet::BitStream& stream, const char* string);
    void writeStringToBitstream16(RakNet::BitStream& stream, const char* string);
    void writeStringToBitstream32(RakNet::BitStream& stream, const std::string& string);
    void writeStringToBitstream16(RakNet::BitStream& stream, const std::string& string);
    char* readStringFromBitstream32(RakNet::BitStream& stream);
    char* readStringFromBitstream16(RakNet::BitStream& stream);
    std::string readStringFromBitstream32S(RakNet::BitStream& stream);
    std::string readStringFromBitstream16S(RakNet::BitStream& stream);
    struct PacketCallback {
      std::function<
        void(
          uint8_t packetType,
          uint8_t* body, size_t length,
          RakNet::Time t,
          RakNet::Packet* p
        )
      > call;
      int timesLeft;
    };
    struct LPacketCallback {
      std::function<
        void(
          uint16_t lPacketType, uint32_t userID,
          uint8_t* lbody, size_t llength,
          RakNet::Time t,
          RakNet::Packet* p
        )
      > call;
      int timesLeft;
    };
  }
}