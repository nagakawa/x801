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
  // TODO implement this bloody thing
  /*peer = RakNet::RakPeerInterface::GetInstance();
  RakNet::SocketDescriptor socket(port, 0);
  peer->Startup(maxConnections, &socket, 1);
  peer->SetMaximumIncomingConnections(maxConnections);*/
  (void) maxConnections;
  (void) port;
  (void) ipAddress;
  (void) peer;
}

x801::game::Client::~Client() {
  // nothing for now
}
