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
#include "window/Patcher.h"
#include "window/patcher_views/MapView.h"
#include "window/patcher_views/MobInfoView.h"
#include "window/patcher_views/ModelView.h"
#include "window/patcher_views/TextureView.h"
#include "window/patcher_views/PartView.h"
#include "window/patcher_views/BlueprintView.h"

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
        g.c = this;
        initialise();
      }
      ~Client();
      Client(const Client& s) = delete;
      void operator=(const Client& s) = delete;
      const uint16_t port;
      std::string getIPAddress() const { return ipAddress; }
      bool isDone() volatile { return done; }
      void login(Credentials& c, PacketCallback loginCallback);
      void login(Credentials& c);
      void sendChatMessage(const char* message);
      void listen();
      void listenConcurrent();
      std::thread& getListenThread() { return listenThread; }
      std::string getUsername(uint32_t id);
      bool getServerAddress(RakNet::SystemAddress& out) const;
      void setDebug(bool d) { debug = d; }
    private:
      void initialise();
      bool readConfig();
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
      void processFilehostURIResponse(
        uint8_t packetType,
        uint8_t* body, size_t length,
        RakNet::Packet* p
      );
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
      void processMobs(
        uint16_t lPacketType,
        uint8_t* lbody, size_t llength,
        RakNet::Packet* p
      );
      void sendKeyInput(const KeyInput& input);
      void switchAreaToCurrent();
      RakNet::RakPeerInterface* peer = nullptr;
      std::string ipAddress;
      bool useIPV6;
      std::multimap<uint8_t, PacketCallback> callbacks;
      std::multimap<uint16_t, LPacketCallback> lCallbacks;
      mutable std::mutex callbackLock;
      mutable std::mutex lCallbackLock;
      char* publicKey = nullptr;
      // GameState g;
      ClientWindow* cw = nullptr;
      ClientGameState g;
      volatile bool done = false;
      uint8_t* cookie = nullptr;
      Credentials cred;
      std::thread windowThread;
      std::thread listenThread;
      RakNet::Time drift = 0;
      Patcher* patcher = nullptr;
      TextureView* textureView = nullptr;
      ModelView* modelView = nullptr;
      MapView* mapView = nullptr;
      PartView* partView = nullptr;
      BlueprintView* blueprintView = nullptr;
      MobInfoView* mobInfoView = nullptr;
      bool debug = false;
      int width = 1280;
      int height = 960;
      friend class ClientWindow;
      friend class TerrainRenderer;
      friend class EntityRenderer;
    };
  }
}