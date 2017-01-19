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

#include <algorithm>
#include <iostream>

void x801::game::Client::initialise() {
  peer = RakNet::RakPeerInterface::GetInstance();
  RakNet::SocketDescriptor clientSocket(0, 0);
  clientSocket.socketFamily = AF_INET;
  peer->Startup(1, &clientSocket, 1);
  peer->SetOccasionalPing(true);
  auto status = peer->Connect(ipAddress.c_str(), port, nullptr, 0);
  if (status != RakNet::CONNECTION_ATTEMPT_STARTED) {
    throw "Connection failed...";
  }
}

void x801::game::Client::handlePacket(
    uint8_t packetType,
    uint8_t* body, size_t length,
    RakNet::Packet* p) {
  (void) body; (void) length; (void) p;
  switch (packetType) {
  case ID_NO_FREE_INCOMING_CONNECTIONS:
    std::cout << "The server is full.\n";
    break;
  case ID_CONNECTION_LOST:
  case ID_DISCONNECTION_NOTIFICATION:
    std::cout << "Connection lost.\n";
    break;
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
void x801::game::Client::handleLPacket(
    uint16_t lPacketType,
    uint8_t* lbody, size_t llength,
    RakNet::Packet* p) {
  // TODO implement
  (void) lbody; (void) llength; (void) p;
  switch (lPacketType) {
    //
  }
  auto range = lCallbacks.equal_range(lPacketType);
  for_each(
    range.first, range.second,
    [lPacketType, lbody, llength, p](auto& pair) {
      pair.second.call(lPacketType, nullptr, lbody, llength, p);
      --pair.second.timesLeft;
    }
  );
}

void x801::game::Client::listen() {
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
        uint8_t* lbody = body + 2;
        size_t llength = length - 2;
        handleLPacket(lpacketType, lbody, llength, p);
      } else {
        handlePacket(packetType, body, length, p);
      }
    }
  }
}

x801::game::Client::~Client() {
  RakNet::RakPeerInterface::DestroyInstance(peer);
}
