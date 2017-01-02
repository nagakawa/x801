#include "packet.h"

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

uint8_t x801::game::getPacketType(RakNet::Packet* p) {
  if (p->data[0] == ID_TIMESTAMP)
    return p->data[sizeof(uint8_t) + sizeof(RakNet::TimeMS)];
  return p->data[0];
}

size_t x801::game::getPacketOffset(RakNet::Packet* p) {
  if (p->data[0] == ID_TIMESTAMP)
    return 2 * sizeof(uint8_t) + sizeof(RakNet::TimeMS);
  return 1;
}