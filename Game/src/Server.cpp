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
  socket.socketFamily = AF_INET;
  peer->Startup(maxConnections, &socket, 1);
  peer->SetMaximumIncomingConnections(maxConnections);
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
    RakNet::Packet* p) {
  (void) body; (void) length;
  switch (packetType) {
  case ID_CONNECTION_LOST:
  case ID_DISCONNECTION_NOTIFICATION: {
      // gracefully log out player
      uint32_t playerID = playersByAddress[p->systemAddress];
      logout(playerID);
      playersByAddress.erase(p->systemAddress);
    }
  case PACKET_MOTD: {
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
  }
  auto range = callbacks.equal_range(packetType);
  for_each(
    range.first, range.second,
    [packetType, body, length, p](auto& pair) {
      pair.second.call(packetType, body, length, p);
      --pair.second.timesLeft;
    }
  );
}
void x801::game::Server::handleLPacket(
    uint16_t lPacketType, uint8_t* cookie,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p) {
  // TODO implement
  (void) lbody; (void) llength; (void) p;
  std::array<uint8_t, COOKIE_LEN> cookieAsArray;
  for (int i = 0; i < COOKIE_LEN; ++i) cookieAsArray[i] = cookie[i];
  uint32_t playerID = playersByCookie[cookieAsArray];
  (void) playerID;
  switch (lPacketType) {
    //
  }
  auto range = lCallbacks.equal_range(lPacketType);
  for_each(
    range.first, range.second,
    [lPacketType, cookie, lbody, llength, p](auto& pair) {
      pair.second.call(lPacketType, cookie, lbody, llength, p);
      --pair.second.timesLeft;
    }
  );
}

void x801::game::Server::listen() {
  while (true) {
    for (
        RakNet::Packet* p = peer->Receive();
        p != nullptr;
        peer->DeallocatePacket(p), p = peer->Receive()) {
      uint8_t packetType = getPacketType(p);
      size_t offset = getPacketOffset(p);
      uint8_t* body = p->data + offset;
      size_t length = p->length - offset;
      if (packetType == PACKET_IM_LOGGED_IN) {
        uint16_t lpacketType = (body[0] << 8) | body[1];
        uint8_t* cookie = body + 2;
        uint8_t* lbody = body + 10;
        size_t llength = length - 10;
        handleLPacket(lpacketType, cookie, lbody, llength, p);
      } else {
        handlePacket(packetType, body, length, p);
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