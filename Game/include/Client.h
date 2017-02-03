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
#include <thread>
#include <unordered_map>
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <SecureHandshake.h>
namespace x801 {
  namespace game {
    class Client;
  }
}
#include "Credentials.h"
#include "GameState.h"
#include "KeyInput.h"
#include "packet.h"
#include "window/ClientWindow.h"

namespace x801 {
  namespace game {
    class Client {
    public:
      Client(
          std::string ipAddress,
          uint16_t port,
          bool useIPV6 = false
      ) : port(port),
          ipAddress(ipAddress),
          useIPV6(useIPV6) {
        initialise();
      }
      ~Client();
      Client(const Client& s) = delete;
      void operator=(const Client& s) = delete;
      const uint16_t port;
      std::string getIPAddress() const { return ipAddress; }
      bool isDone() { return done; }
      void login(Credentials& c, PacketCallback loginCallback);
      void login(Credentials& c);
      void sendChatMessage(const char* message);
      void listen();
      void listenConcurrent();
      std::thread& getListenThread() { return listenThread; }
      std::string getUsername(uint32_t id);
    private:
      void initialise();
      bool handlePacket(
        uint8_t packetType,
        uint8_t* body, size_t length,
        RakNet::Time t,
        RakNet::Packet* p
      );
      bool handleLPacket(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Time t,
        RakNet::Packet* p
      );
      void requestMOTD();
      void requestMOTD(PacketCallback motdCallback);
      void openWindow();
      void openWindowConcurrent();
      void sendLoginPacket(PacketCallback loginCallback);
      void requestUsernames(size_t count, uint32_t* ids);
      void requestUsername(uint32_t id);
      void processUsernameResponse(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Packet* p
      );
      void processChatMessageCode(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Packet* p
      );
      void processChatMessage(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Packet* p
      );
      void processMovement(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Time t,
        RakNet::Packet* p
      );
      RakNet::RakPeerInterface* peer = nullptr;
      std::string ipAddress;
      bool useIPV6;
      std::multimap<uint8_t, PacketCallback> callbacks;
      std::multimap<uint16_t, LPacketCallback> lCallbacks;
      char* publicKey = nullptr;
      // GameState g;
      ClientWindow* cw = nullptr;
      ClientGameState g;
      volatile bool done = false;
      uint8_t* cookie = nullptr;
      Credentials cred;
      std::thread windowThread;
      std::thread listenThread;
      friend class ClientWindow;
    };
  }
}