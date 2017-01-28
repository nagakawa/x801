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
#include <iostream>
#include <GLFW/glfw3.h>
#include <BitStream.h>
#include <SecureHandshake.h>

void x801::game::Client::initialise() {
  peer = RakNet::RakPeerInterface::GetInstance();
  RakNet::SocketDescriptor clientSocket(0, 0);
  clientSocket.socketFamily = useIPV6 ? AF_INET6 : AF_INET;
  peer->Startup(1, &clientSocket, 1);
  peer->SetOccasionalPing(true);
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
      RakNet::Packet* p) {
        (void) packetType; (void) body; (void) length; (void) p;
        std::cout << "You have been disconnected.\n";
      }, -1
  };
  callbacks.insert({ID_CONNECTION_LOST, logoutCallback});
  callbacks.insert({ID_DISCONNECTION_NOTIFICATION, logoutCallback});
  PacketCallback connectCallback = {
    [this](
      uint8_t packetType,
      uint8_t* body, size_t length,
      RakNet::Packet* p) {
        (void) packetType; (void) body; (void) length; (void) p;
        this->requestMOTD();
      }, -1
  };
  callbacks.insert({ID_CONNECTION_REQUEST_ACCEPTED, connectCallback});
  listenConcurrent();
}

bool x801::game::Client::handlePacket(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Packet* p) {
  (void) body; (void) length; (void) p;
  std::cerr << "It's a packet! ID = " << (int) packetType << "\n";
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
  auto range = callbacks.equal_range(packetType);
  for (auto iterator = range.first; iterator != range.second;) {
    if (iterator->first != packetType) {
      ++iterator;
      continue;
    }
    (iterator->second.call)(packetType, body, length, p);
    if (iterator->second.timesLeft != -1) --iterator->second.timesLeft;
    if (iterator->second.timesLeft == 0) iterator = callbacks.erase(iterator);
    else ++iterator;
  }
  return true;
}
bool x801::game::Client::handleLPacket(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p) {
  // TODO implement
  (void) lbody; (void) llength; (void) p;
  std::cerr << "It's an lpacket!\n";
  switch (lPacketType) {
    //
  }
  auto range = lCallbacks.equal_range(lPacketType);
  for (auto iterator = range.first; iterator != range.second;) {
    if (iterator->first != lPacketType) {
      ++iterator;
      continue;
    }
    (iterator->second.call)(lPacketType, 0, lbody, llength, p);
    if (iterator->second.timesLeft != -1) --iterator->second.timesLeft;
    if (iterator->second.timesLeft == 0) iterator = lCallbacks.erase(iterator);
    else ++iterator;
  }
  return true;
}

void x801::game::Client::listen() {
  std::cerr << "Listening...\n";
  bool shouldContinue = true;
  while (shouldContinue) {
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
        uint8_t* lbody = body + 2;
        size_t llength = length - 2;
        shouldContinue = handleLPacket(lpacketType, lbody, llength, p);
      } else {
        shouldContinue = handlePacket(packetType, body, length, p);
      }
    }
  }
  if (cw != nullptr) glfwSetWindowShouldClose(cw->underlying(), true);
}

void x801::game::Client::listenConcurrent() {
  listenThread = std::thread([this]() { this->listen(); });
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
      RakNet::Packet* p) {
        (void) packetType; (void) p;
        RakNet::BitStream stream(body, length, false);
        const char* s = readStringFromBitstream32(stream);
        std::cout << "MOTD:\n";
        std::cout << s << '\n';
        delete[] s;
      }, 1
  };
  requestMOTD(motdCallback);
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
      RakNet::Packet* p) {
        (void) p; (void) packetType;
        uint8_t stat = body[0];
        if (length < 17) stat = LOGIN_NOT_ENOUGH_DATA;
        if (stat == LOGIN_OK) {
          this->cookie = new uint8_t[RAW_HASH_LENGTH];
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
      RakNet::Packet* p) {
        (void) packetType; (void) body; (void) length; (void) p;
        this->sendLoginPacket(loginCallback);
      }, -1
  };
  callbacks.insert({ID_CONNECTION_REQUEST_ACCEPTED, connectCallback});
}

void x801::game::Client::openWindow() {
  cw = new ClientWindow(1280, 960, 0, 0, "Experiment801", 3, 3, false);
  cw->c = this;
  cw->start();
}

void x801::game::Client::openWindowConcurrent() {
  windowThread = std::thread([this]() { this->openWindow(); });
}

x801::game::Client::~Client() {
  done = true;
  RakNet::RakPeerInterface::DestroyInstance(peer);
  delete[] publicKey;
}
