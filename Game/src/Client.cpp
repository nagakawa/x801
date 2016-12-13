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

x801::game::Client::~Client() {
  RakNet::RakPeerInterface::DestroyInstance(peer);
}
