#include "Client.h"

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

using namespace x801::game;

#include <stdlib.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <ratio>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <GLFW/glfw3.h>
#include <BitStream.h>
#include <SecureHandshake.h>
#include <portable_endian.h>
#include "Server.h"

void x801::game::Client::initialise() {
  readConfig();
  peer = RakNet::RakPeerInterface::GetInstance();
  peer->SetOccasionalPing(true);
  RakNet::SocketDescriptor clientSocket(0, 0);
  clientSocket.socketFamily = useIPV6 ? AF_INET6 : AF_INET;
  peer->Startup(1, &clientSocket, 1);
  RakNet::PublicKey pk;
  pk.publicKeyMode = RakNet::PKM_ACCEPT_ANY_PUBLIC_KEY;
  publicKey = new char[cat::EasyHandshake::PUBLIC_KEY_BYTES];
  pk.remoteServerPublicKey = publicKey;
  auto status = peer->Connect(ipAddress.c_str(), port, nullptr, 0, &pk);
  if (status != RakNet::CONNECTION_ATTEMPT_STARTED) {
    throw "Connection failed...";
  }
  // set packet callbacks
  PacketCallback logoutCallback = {
    [](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) packetType; (void) body; (void) length; (void) t; (void) p;
        std::cout << "You have been disconnected.\n";
      }, -1
  };
  callbacks.insert({ID_CONNECTION_LOST, logoutCallback});
  callbacks.insert({ID_DISCONNECTION_NOTIFICATION, logoutCallback});
  PacketCallback connectCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) packetType; (void) body; (void) length; (void) t; (void) p;
        this->requestMOTD();
      }, -1
  };
  callbacks.insert({ID_CONNECTION_REQUEST_ACCEPTED, connectCallback});
  PacketCallback filehostCallback =
    MAKE_PACKET_CALLBACK(processFilehostURIResponse, -1);
  callbacks.insert({PACKET_FILE, filehostCallback});
  LPacketCallback idCallback =
    MAKE_LPACKET_CALLBACK_CLIENT(processUsernameResponse, -1);
  lCallbacks.insert({LPACKET_IDENTIFY, idCallback});
  LPacketCallback chatCallback1 =
    MAKE_LPACKET_CALLBACK_CLIENT(processChatMessageCode, -1);
  lCallbacks.insert({LPACKET_CHAT, chatCallback1});
  LPacketCallback chatCallback2 =
    MAKE_LPACKET_CALLBACK_CLIENT(processChatMessage, -1);
  lCallbacks.insert({LPACKET_RECEIVE_CHAT, chatCallback2});
  LPacketCallback moveCallback =
    MAKE_LPACKET_CALLBACK_CLIENT_TIMED(processMovement, -1);
  lCallbacks.insert({LPACKET_MOVE, moveCallback});
  listenConcurrent();
}

static const char* CONFIG_PATH = "intrinsic-assets/config.json";

bool x801::game::Client::readConfig() {
  // Try to read
  if (!boost::filesystem::exists(CONFIG_PATH) ||
      boost::filesystem::is_directory(CONFIG_PATH)) {
    std::cerr <<
      "Error: " << CONFIG_PATH <<
      " doesn't exist or is a directory.\n" <<
      "Aborting and using defaults.\n";
    return false;
  }
  boost::filesystem::ifstream input(
    CONFIG_PATH,
    std::ios_base::in | std::ios_base::binary
  );
  std::string s = x801::base::slurp(input);
  rapidjson::Document config;
  rapidjson::ParseResult stat = config.Parse(s.c_str());
  if (stat.IsError()) {
    std::cerr << "Error when reading config file: " <<
      rapidjson::GetParseError_En(stat.Code()) << '\n'
      << "  at offset " << stat.Offset() << '\n';
    return false;
  }
  if (!config.IsObject()) {
    std::cerr << "Error: Config file must contain a JSON object.\n";
    return false;
  }
  auto endNode = config.MemberEnd();
  // resolution
  auto resolutionNode = config.FindMember("resolution");
  if (resolutionNode != endNode) {
    if (!resolutionNode->value.IsArray() ||
        resolutionNode->value.GetArray().Size() != 2) {
      std::cerr << "Warning: member 'resolution' of config must be " <<
        "a two-element array of integers." <<
        "Ignoring this attribute.\n";
    } else {
      const auto& wnode = resolutionNode->value[0];
      const auto& hnode = resolutionNode->value[1];
      if (!wnode.IsInt() || !hnode.IsInt()) {
        std::cerr << "Warning: member 'resolution' of config must be " <<
          "a two-element array of integers." <<
          "Ignoring this attribute.\n";
      } else {
        width = wnode.GetInt();
        height = hnode.GetInt();
      }
    }
  }
  return true;
}

bool x801::game::Client::handlePacket(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Time t,
    RakNet::Packet* p) {
  (void) body; (void) length; (void) p;
  switch (packetType) {
  case ID_NO_FREE_INCOMING_CONNECTIONS:
    std::cout << "The server is full.\n";
    return false;
  case ID_CONNECTION_LOST:
  case ID_DISCONNECTION_NOTIFICATION:
    std::cout << "Connection lost.\n";
    return false;
  case ID_CONNECTION_ATTEMPT_FAILED:
    std::cout << "Failed to connect to server.\n";
    return false;
  }
  callbackLock.lock();
  auto range = callbacks.equal_range(packetType);
  for (auto iterator = range.first; iterator != range.second;) {
    if (iterator->first != packetType) {
      ++iterator;
      continue;
    }
    (iterator->second.call)(packetType, body, length, t, p);
    if (iterator->second.timesLeft != -1) --iterator->second.timesLeft;
    if (iterator->second.timesLeft == 0) iterator = callbacks.erase(iterator);
    else ++iterator;
  }
  callbackLock.unlock();
  return true;
}
bool x801::game::Client::handleLPacket(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Time t,
    RakNet::Packet* p) {
  // TODO implement
  (void) lbody; (void) llength; (void) p;
  lCallbackLock.lock();
  auto range = lCallbacks.equal_range(lPacketType);
  for (auto iterator = range.first; iterator != range.second;) {
    if (iterator->first != lPacketType) {
      ++iterator;
      continue;
    }
    (iterator->second.call)(lPacketType, 0, lbody, llength, t, p);
    if (iterator->second.timesLeft != -1) --iterator->second.timesLeft;
    if (iterator->second.timesLeft == 0) iterator = lCallbacks.erase(iterator);
    else ++iterator;
  }
  lCallbackLock.unlock();
  return true;
}

static const double GAP = 10e-3;
static const double THRESH = 2e-3;

void x801::game::Client::listen() {
  std::cerr << "Listening...\n";
  bool shouldContinue = true;
  auto lastTime = std::chrono::steady_clock::now();
  while (shouldContinue) {
    for (
        RakNet::Packet* p = peer->Receive();
        p != nullptr;
        peer->DeallocatePacket(p), p = peer->Receive()) {
      RakNet::Time t = 0;
      if (p->data[0] == ID_TIMESTAMP) {
        RakNet::BitStream s(p->data + 1, sizeof(RakNet::Time), false);
        s.Read(t);
      }
      uint8_t packetType = getPacketType(p);
      size_t offset = getPacketOffset(p);
      uint8_t* body = p->data + offset;
      size_t length = p->length - offset;
      if (packetType == PACKET_IM_LOGGED_IN) {
        uint16_t lpacketType = (body[0] << 8) | body[1];
        uint8_t* lbody = body + 2;
        size_t llength = length - 2;
        shouldContinue = handleLPacket(lpacketType, lbody, llength, t, p);
      } else {
        shouldContinue = handlePacket(packetType, body, length, t, p);
      }
    }
    std::chrono::time_point<std::chrono::steady_clock> thisTime;
    double diff = 0;
    while (diff < GAP) {
      thisTime = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::ratio<1, 1>> diffd = (thisTime - lastTime);
      diff = diffd.count();
      if (diff < GAP && diff >= GAP - THRESH)
        std::this_thread::sleep_for(diffd / 2);
    }
    if (cw != nullptr && (cw->underlying() == nullptr ||
        glfwWindowShouldClose(cw->underlying()))) {
      shouldContinue = false;
    }
    lastTime = thisTime;
  }
  if (cw != nullptr && cw->underlying() != nullptr)
    glfwSetWindowShouldClose(cw->underlying(), true);
}

void x801::game::Client::listenConcurrent() {
  listenThread = std::thread([this]() { this->listen(); });
}

bool x801::game::Client::getServerAddress(RakNet::SystemAddress& out) const {
  uint16_t n = 1;
  (void) peer->GetConnectionList(&out, &n);
  return n >= 1;
}

void x801::game::Client::requestMOTD(PacketCallback motdCallback) {
  RakNet::BitStream stream;
  stream.Write(static_cast<uint8_t>(PACKET_MOTD));
  callbacks.insert({PACKET_MOTD, motdCallback});
  peer->Send(
    &stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
    RakNet::UNASSIGNED_RAKNET_GUID, true
  );
}

void x801::game::Client::requestMOTD() {
  PacketCallback motdCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) packetType; (void) p; (void) t;
        RakNet::BitStream stream(body, length, false);
        const char* s = readStringFromBitstream32(stream);
        std::cout << "MOTD:\n";
        std::cout << s << '\n';
        delete[] s;
      }, 1
  };
  requestMOTD(motdCallback);
}

void x801::game::Client::requestUsernames(
    size_t count, uint32_t* ids) {
  size_t totalCount = count + g.totalRequested();
  uint32_t* alreadyRequestedIDs = new uint32_t[totalCount - count];
  g.populateRequested(alreadyRequestedIDs, totalCount - count);
  for (size_t i = 0; i < count; ++i)
    g.addRequest(ids[i]);
  RakNet::BitStream stream;
  stream.Write(static_cast<uint8_t>(PACKET_IM_LOGGED_IN));
  stream.Write(static_cast<uint16_t>(LPACKET_IDENTIFY));
  stream.Write((const char*) cookie, COOKIE_LEN);
  stream.Write(static_cast<uint16_t>(totalCount));
  std::cerr << "Sending requests...\n";
  for (size_t i = 0; i < count; ++i) {
    std::cerr << "Requesting name of user #" << ids[i] << '\n';
    stream.Write(ids[i]);
  }
  for (size_t i = 0; i < totalCount - count; ++i) {
    std::cerr << "Requesting name of user #" << ids[i] << '\n';
    stream.Write(alreadyRequestedIDs[i]);
  }
  peer->Send(
    &stream, HIGH_PRIORITY, RELIABLE_ORDERED, 1,
    RakNet::UNASSIGNED_RAKNET_GUID, true
  );
  delete[] alreadyRequestedIDs;
}

void x801::game::Client::requestUsername(uint32_t id) {
  requestUsernames(1, &id);
}

void x801::game::Client::processUsernameResponse(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p) {
  (void) lPacketType; (void) p;
  RakNet::BitStream stream(lbody, llength, false);
  uint16_t count;
  stream.Read(count);
  g.lookupMutex.lock();
  for (size_t i = 0; i < count; ++i) {
    uint32_t userID;
    stream.Read(userID);
    std::string username = readStringFromBitstream16S(stream);
    std::cerr << i << ") " << userID << ": " << username << '\n';
    g.addUserUnsynchronised(userID, username);
  }
  g.lookupMutex.unlock();
}

std::string x801::game::Client::getUsername(uint32_t id) {
  if (id == 0) return "Server";
  auto it = g.findUsernameByID(id);
  if (it != g.endOfUsernameMap()) return it->second;
  else if (!g.isIDRequested(id)) requestUsername(id);
  std::stringstream ss;
  ss << "#" << id;
  std::string s = ss.str();
  return s;
}

void x801::game::Client::sendChatMessage(const char* message) {
  RakNet::BitStream stream;
  stream.Write(static_cast<uint8_t>(PACKET_IM_LOGGED_IN));
  stream.Write(static_cast<uint16_t>(LPACKET_CHAT));
  stream.Write((const char*) cookie, COOKIE_LEN);
  writeStringToBitstream16(stream, message);
  peer->Send(
    &stream, HIGH_PRIORITY, RELIABLE_ORDERED, 1,
    RakNet::UNASSIGNED_RAKNET_GUID, true
  );
  textureView->getTexture("textures/terrain/blocks.png");
  modelView->getMAI();
  modelView->getMFI();
}

void x801::game::Client::processFilehostURIResponse(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Packet* p) {
  (void) p; (void) packetType;
  RakNet::BitStream input(body, length, false);
  patcher = new Patcher(readStringFromBitstream16S(input), ipAddress);
  patcher->startFetchThread();
  textureView = new TextureView(patcher);
  modelView = new ModelView(patcher);
  mapView = new MapView(patcher);
  partView = new PartView(patcher);
  blueprintView = new BlueprintView(patcher);
}

static const char* statusMessages[] = {
  "Why the fuck are you complaining?",
  "You don't have perimission to execute this command.",
  "You are muted.",
};

void x801::game::Client::processChatMessageCode(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p)  {
  (void) lPacketType; (void) p;
  RakNet::BitStream stream(lbody, llength, false);
  uint8_t status;
  stream.Read(status);
  std::string details = readStringFromBitstream16S(stream);
  if (details.length() != 0)
    cw->getChatWindow()->pushMessage(0, details);
  else if (status != CHAT_OK)
    cw->getChatWindow()->pushMessage(0, statusMessages[status]);
}

void x801::game::Client::processChatMessage(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p)  {
  (void) lPacketType; (void) p;
  RakNet::BitStream stream(lbody, llength, false);
  uint32_t playerID;
  stream.Read(playerID);
  std::string message = readStringFromBitstream16S(stream);
  cw->getChatWindow()->pushMessage(playerID, message);
}
#include <stdio.h>
void x801::game::Client::processMovement(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Time t,
    RakNet::Packet* p) {
  (void) lPacketType; (void) p;
  if (t == 0) return;
  RakNet::BitStream stream(lbody, llength, false);
  uint32_t playerCount;
  stream.Read(playerCount);
  for (size_t i = 0; i < playerCount; ++i) {
    uint32_t playerID;
    int32_t xfix, yfix, tfix;
    stream.Read(playerID);
    stream.Read(xfix);
    stream.Read(yfix);
    stream.Read(tfix);
    Player& p = g.getPlayer(playerID);
    Location& l = p.getLocation();
    l.x = xfix / 65536.0f;
    l.y = yfix / 65536.0f;
    l.rot = 2 * M_PI * tfix / (65536.0f * 65536.0f);
    // std::cerr << playerID << " " <<  xfix << " " << yfix << " " << tfix << '\n';
  }
  g.selfPositionMutex.lock();
  g.locationMutex.lock_shared();
  auto it = g.playersByID.find(g.myID);
  if (it != g.playersByID.end()) {
    g.selfPosition = g.playersByID[g.myID].getLocation();
  }
  g.locationMutex.unlock_shared();
  g.fastForwardSelf(t);
  g.selfPositionMutex.unlock();
}

void x801::game::Client::sendLoginPacket(PacketCallback loginCallback) {
  RakNet::BitStream stream;
  stream.Write(static_cast<uint8_t>(PACKET_LOGIN));
  writeStringToBitstream16(stream, cred.getUsername());
  stream.Write((char*) cred.getHash(), RAW_HASH_LENGTH);
  callbacks.insert({PACKET_LOGIN, loginCallback});
  peer->Send(
    &stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
    RakNet::UNASSIGNED_RAKNET_GUID, true
  );
}

static const char* loginStatusMessages[] = {
  "Login was OK, why the fuck are you complaining?",
  "Username or password was wrong.",
  "Server is full.",
  "You are banned from this server.",
  "This user is already logged in.",
  "Unknown login error.",
  "The server did not send enough bytes for a cookie.",
};

void x801::game::Client::login(Credentials& c) {
  PacketCallback loginCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) p; (void) packetType; (void) t;
        RakNet::BitStream stream(body, length, false);
        uint8_t stat;
        stream.Read(stat);
        if (length < (1 + COOKIE_LEN + sizeof(uint32_t))
            && stat == LOGIN_OK)
          stat = LOGIN_NOT_ENOUGH_DATA;
        if (stat == LOGIN_OK) {
          this->cookie = new uint8_t[COOKIE_LEN];
          stream.Read((char*) this->cookie, COOKIE_LEN);
          uint32_t id;
          stream.Read(id);
          this->g.setID(id);
          openWindowConcurrent();
          memcpy(this->cookie, body + 1, 16);
        } else {
          std::cerr << "Login failed!\n";
          std::cerr << loginStatusMessages[stat] << '\n';
          exit(-1);
        }
      }, 1
  };
  login(c, loginCallback);
}

void x801::game::Client::login(Credentials& c, PacketCallback loginCallback) {
  cred = c;
  PacketCallback connectCallback = {
    [this, loginCallback](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) packetType; (void) body; (void) length; (void) t; (void) p;
        this->sendLoginPacket(loginCallback);
      }, -1
  };
  callbacks.insert({ID_CONNECTION_REQUEST_ACCEPTED, connectCallback});
}

void x801::game::Client::sendKeyInput(const KeyInput& input) {
  RakNet::BitStream stream;
  stream.Write(static_cast<uint8_t>(ID_TIMESTAMP));
  stream.Write(input.time);
  stream.Write(static_cast<uint8_t>(PACKET_IM_LOGGED_IN));
  stream.Write(static_cast<uint16_t>(LPACKET_MOVE));
  stream.Write((char*) cookie, COOKIE_LEN);
  stream.Write(input.inputs);
  peer->Send(
    &stream, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0,
    RakNet::UNASSIGNED_RAKNET_GUID, true
  );
}

void x801::game::Client::openWindow() {
  glfwInit();
  if (debug)
    cw = new ClientWindow(width, height, 0, 0, "Experiment801 (live debug)", 4, 5, true);
  else
    cw = new ClientWindow(width, height, 0, 0, "Experiment801", 3, 3, false);
  cw->c = this;
  cw->start();
  glfwTerminate();
}

void x801::game::Client::openWindowConcurrent() {
  windowThread = std::thread([this]() { this->openWindow(); });
}

x801::game::Client::~Client() {
  done = true;
  if (listenThread.joinable()) listenThread.join();
  if (windowThread.joinable()) windowThread.join();
  patcher->stopFetchThread();
  RakNet::RakPeerInterface::DestroyInstance(peer);
  delete cw;
  delete[] publicKey;
  delete[] cookie;
  delete patcher;
  delete textureView;
  delete modelView;
  delete partView;
  delete blueprintView;
}
