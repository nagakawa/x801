#include "Server.h"

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

#include <algorithm>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <BitStream.h>
#include <RakNetTypes.h>
#include "packet.h"

static const char* SERVER_MOTD =
  "This is the default server MOTD (message of the day).\n"
  "Currently this is hardcoded into the application, but eventually\n"
  "you'll be able to change it through configuration files."
  ;

void x801::game::Server::initialise() {
  peer = RakNet::RakPeerInterface::GetInstance();
  updateKeyFiles();
  RakNet::SocketDescriptor socket(port, 0);
  socket.socketFamily = useIPV6 ? AF_INET6 : AF_INET;
  peer->Startup(maxConnections, &socket, 1);
  peer->SetMaximumIncomingConnections(maxConnections);
  // set packet callbacks
  PacketCallback logoutCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) t;
        this->logoutByPacket(packetType, body, length, p);
      }, -1
  };
  callbacks.insert({ID_CONNECTION_LOST, logoutCallback});
  callbacks.insert({ID_DISCONNECTION_NOTIFICATION, logoutCallback});
  PacketCallback motdCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) t;
        this->sendMOTD(packetType, body, length, p);
      }, -1
  };
  callbacks.insert({PACKET_MOTD, motdCallback});
  PacketCallback loginCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) t;
        this->processLogin(packetType, body, length, p);
      }, -1
  };
  callbacks.insert({PACKET_LOGIN, loginCallback});
  LPacketCallback usernameCallback = {
    [this](
      uint16_t lPacketType, uint32_t userID,
      uint8_t* lbody, size_t llength,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) t;
        this->processUsernameRequest(lPacketType, userID, lbody, llength, p);
      }, -1
  };
  lCallbacks.insert({LPACKET_IDENTIFY, usernameCallback});
  LPacketCallback chatCallback = {
    [this](
      uint16_t lPacketType, uint32_t userID,
      uint8_t* lbody, size_t llength,
      RakNet::Time t,
      RakNet::Packet* p) {
        (void) t;
        this->processChatRequest(lPacketType, userID, lbody, llength, p);
      }, -1
  };
  lCallbacks.insert({LPACKET_CHAT, chatCallback});
  listen();
}

void x801::game::Server::logout(uint32_t playerID) {
  std::array<uint8_t, COOKIE_LEN>& cookie = cookiesByPlayer[playerID];
  playersByCookie.erase(cookie);
  cookiesByPlayer.erase(playerID);
  g.logout(playerID);
}

void x801::game::Server::handlePacket(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Time t,
    RakNet::Packet* p) {
  std::cerr << "It's a packet! ID = " << (int) packetType << "\n";
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
}
void x801::game::Server::handleLPacket(
    uint16_t lPacketType, uint8_t* cookie,
    uint8_t* lbody, size_t llength,
    RakNet::Time t,
    RakNet::Packet* p) {
  // TODO implement
  (void) lbody; (void) llength; (void) p;
  std::cerr << "It's an lpacket! ID = " << lPacketType << "\n";
  std::array<uint8_t, COOKIE_LEN> cookieAsArray;
  for (int i = 0; i < COOKIE_LEN; ++i) cookieAsArray[i] = cookie[i];
  // Not sure why playersByCookie.find(cookieAsArray) returns a
  // garbage iterator when it doesn't find the cookie. Might be
  // a bug with the STL on my computer.
  if (playersByCookie.count(cookieAsArray) == 0) {
    // invalid cookie
    sendUnrecognisedCookiePacket(p);
  }
  uint32_t playerID = playersByCookie[cookieAsArray];
  auto range = lCallbacks.equal_range(lPacketType);
  for (auto iterator = range.first; iterator != range.second;) {
    if (iterator->first != lPacketType) {
      ++iterator;
      continue;
    }
    (iterator->second.call)(lPacketType, playerID, lbody, llength, t, p);
    if (iterator->second.timesLeft != -1) --iterator->second.timesLeft;
    if (iterator->second.timesLeft == 0) iterator = lCallbacks.erase(iterator);
    else ++iterator;
  }
}

void x801::game::Server::listen() {
  while (true) {
    for (
        RakNet::Packet* p = peer->Receive();
        p != nullptr;
        peer->DeallocatePacket(p), p = peer->Receive()) {
      RakNet::Time t = 0;
      if (p->data[0] == ID_TIMESTAMP) {
        RakNet::BitStream s(p->data, 1, sizeof(RakNet::Time));
        s.Read(t);
      }
      uint8_t packetType = getPacketType(p);
      size_t offset = getPacketOffset(p);
      uint8_t* body = p->data + offset;
      size_t length = p->length - offset;
      if (packetType == PACKET_IM_LOGGED_IN) {
        uint16_t lpacketType = (body[0] << 8) | body[1];
        uint8_t* cookie = body + 2;
        uint8_t* lbody = body + 2 + COOKIE_LEN;
        size_t llength = length - 2 - COOKIE_LEN;
        if ((ssize_t) llength < 0)
          sendUnrecognisedCookiePacket(p);
        else
          handleLPacket(lpacketType, cookie, lbody, llength, t, p);
      } else {
        handlePacket(packetType, body, length, t, p);
      }
    }
  }
}

x801::game::Server::~Server() {
  if (publicKey != nullptr) delete[] publicKey;
  if (privateKey != nullptr) delete[] privateKey;
  RakNet::RakPeerInterface::DestroyInstance(peer);
}

const char* x801::game::KEY_DIR = "keys/";
const char* x801::game::KEY_PUBLIC = "keys/public.bin";
const char* x801::game::KEY_PRIVATE = "keys/private.bin";

void x801::game::Server::updateKeyFiles() {
  if (!boost::filesystem::exists(KEY_DIR) ||
      !boost::filesystem::is_directory(KEY_DIR)) {
    std::cout <<
      "Warning: overwriting " << KEY_DIR <<
      " because it is not a directory\n";
    boost::filesystem::remove(KEY_DIR);
    boost::filesystem::create_directories(KEY_DIR);
  }
  int pubSize = cat::EasyHandshake::PUBLIC_KEY_BYTES;
  publicKey = new char[pubSize];
  int privSize = cat::EasyHandshake::PRIVATE_KEY_BYTES;
  privateKey = new char[privSize];
  bool ok = false;
  if (boost::filesystem::exists(KEY_PUBLIC) &&
      !boost::filesystem::is_directory(KEY_PUBLIC)) {
    ok = true;
    boost::filesystem::ifstream input(
      KEY_PUBLIC,
      std::ios_base::in | std::ios_base::binary
    );
    int bytesRead = input.readsome(publicKey, pubSize);
    if (bytesRead < pubSize) ok = false;
  }
  if (boost::filesystem::exists(KEY_PRIVATE) &&
      !boost::filesystem::is_directory(KEY_PRIVATE)) {
    ok = true;
    boost::filesystem::ifstream input(
      KEY_PRIVATE,
      std::ios_base::in | std::ios_base::binary
    );
    int bytesRead = input.readsome(privateKey, privSize);
    if (bytesRead < privSize) ok = false;
  } else {
    ok = false;
  }
  if (!ok) {
    std::cout <<
      "Key files missing or corrupt; [re]generating them\n";
    cat::EasyHandshake handshake;
    handshake.GenerateServerKey(publicKey, privateKey);
    boost::filesystem::remove_all(KEY_PUBLIC);
    boost::filesystem::ofstream output1(
      KEY_PUBLIC,
      std::ios_base::out | std::ios_base::binary
    );
    output1.write(publicKey, pubSize);
    boost::filesystem::remove_all(KEY_PRIVATE);
    boost::filesystem::ofstream output2(
      KEY_PRIVATE,
      std::ios_base::out | std::ios_base::binary
    );
    output2.write(privateKey, privSize);
  }
  peer->InitializeSecurity(publicKey, privateKey, false);
}

void x801::game::Server::logoutByPacket(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Packet* p) {
  (void) packetType; (void) body; (void) length;
  uint32_t playerID = playersByAddress[p->systemAddress];
  logout(playerID);
  playersByAddress.erase(p->systemAddress);
  addressesByPlayer.erase(playerID);
}

void x801::game::Server::sendMOTD(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Packet* p) {
  (void) packetType; (void) body; (void) length;
  RakNet::BitStream stream;
  stream.Write(static_cast<RakNet::MessageID>(PACKET_MOTD));
  writeStringToBitstream32(stream, SERVER_MOTD);
  peer->Send(
    &stream,
    HIGH_PRIORITY, RELIABLE_ORDERED,
    0, p->systemAddress,
    false
  );
}

LoginStatus x801::game::Server::login(
    Credentials& cred, uint32_t& playerID,
    uint8_t* cookie, RakNet::SystemAddress address) {
  LoginStatus stat = g.login(cred, playerID);
  if (stat != LOGIN_OK) return stat;
  x801::base::writeRandomBytes(cookie, COOKIE_LEN);
  std::array<uint8_t, COOKIE_LEN> cookieAsArray;
  for (int i = 0; i < COOKIE_LEN; ++i) cookieAsArray[i] = cookie[i];
  playersByCookie[cookieAsArray] = playerID;
  cookiesByPlayer[playerID] = cookieAsArray;
  playersByAddress[address] = playerID;
  addressesByPlayer[playerID] = address;
  return stat;
}

void x801::game::Server::processLogin(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Packet* p) {
  (void) packetType;
  RakNet::BitStream stream(body, length, false);
  std::string string = readStringFromBitstream16(stream);
  uint8_t hash[RAW_HASH_LENGTH];
  stream.Read((char*) hash, RAW_HASH_LENGTH);
  Credentials cred(string, hash);
  uint8_t output[2 + COOKIE_LEN];
  output[0] = PACKET_LOGIN;
  uint32_t playerID;
  LoginStatus status = login(cred, playerID, output + 2, p->systemAddress);
  output[1] = (uint8_t) status;
  std::cerr << "Login status was " << status << '\n';
  peer->Send(
    (const char*) output, 2 + COOKIE_LEN, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
    p->systemAddress, false
  );
}

void x801::game::Server::processUsernameRequest(
    uint16_t lPacketType, uint32_t userID,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p) {
  (void) lPacketType; (void) userID;
  RakNet::BitStream stream(lbody, llength, false);
  RakNet::BitStream output;
  RakNet::BitStream toutput;
  output.Write(static_cast<uint8_t>(PACKET_IM_LOGGED_IN));
  output.Write(static_cast<uint16_t>(LPACKET_IDENTIFY));
  uint16_t idCount;
  stream.Read(idCount);
  uint16_t netIDCount = idCount;
  for (size_t i = 0; i < idCount; ++i) {
    uint32_t userID;
    stream.Read(userID);
    auto iterator = g.findUsernameByID(userID);
    if (iterator == g.endOfUsernameMap()) {
      --netIDCount;
      continue;
    }
    std::cerr << i << ") " << iterator->first << ": " << iterator->second << '\n';
    toutput.Write(userID);
    writeStringToBitstream16(toutput, iterator->second);
  }
  output.Write(netIDCount);
  output.Write(&toutput);
  peer->Send(
    &output, HIGH_PRIORITY, RELIABLE_ORDERED, 1,
    p->systemAddress, false
  );
}

void x801::game::Server::processChatRequest(
    uint16_t lPacketType, uint32_t userID,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p) {
  (void) lPacketType;
  RakNet::BitStream stream(lbody, llength, false);
  RakNet::BitStream output, output2;
  const char* message = readStringFromBitstream16(stream);
  output.Write(static_cast<uint8_t>(PACKET_IM_LOGGED_IN));
  output.Write(static_cast<uint16_t>(LPACKET_CHAT));
  output.Write(static_cast<uint8_t>(CHAT_OK));
  peer->Send(
    &output, HIGH_PRIORITY, RELIABLE_ORDERED, 1,
    p->systemAddress, false
  );
  output2.Write(static_cast<uint8_t>(PACKET_IM_LOGGED_IN));
  output2.Write(static_cast<uint16_t>(LPACKET_RECEIVE_CHAT));
  output2.Write(userID);
  writeStringToBitstream16(output2, message);
  peer->Send(
    &output2, HIGH_PRIORITY, RELIABLE_ORDERED, 1,
    RakNet::UNASSIGNED_RAKNET_GUID, true
  );
  delete[] message;
}

void x801::game::Server::sendUnrecognisedCookiePacket(RakNet::Packet* p) {
  uint8_t message = PACKET_UNRECOGNISED_COOKIE;
  peer->Send(
    (const char*) &message, 1, MEDIUM_PRIORITY, RELIABLE_ORDERED, 9,
    p->systemAddress, false
  );
}