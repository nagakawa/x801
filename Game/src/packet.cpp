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

#include <utils.h>

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

template <typename Str, typename N>
void writeString(RakNet::BitStream& stream, const Str string) {
  size_t size = x801::base::getLength(string);
  //size = std::min()
  stream.Write(static_cast<N>(size));
  stream.Write(x801::base::getPointer(string), size);
}

void x801::game::writeStringToBitstream32(
  RakNet::BitStream& stream, const char* string) {
  writeString<const char*, uint32_t>(stream, string);
}

void x801::game::writeStringToBitstream16(
  RakNet::BitStream& stream, const char* string) {
  writeString<const char*, uint16_t>(stream, string);
}

void x801::game::writeStringToBitstream32(
  RakNet::BitStream& stream, const std::string& string) {
  writeString<const std::string&, uint32_t>(stream, string);
}

void x801::game::writeStringToBitstream16(
  RakNet::BitStream& stream, const std::string& string) {
  writeString<const std::string&, uint16_t>(stream, string);
}

char* x801::game::readStringFromBitstream32(RakNet::BitStream& stream) {
  uint32_t length;
  stream.Read(length);
  char* c = new char[length + 1];
  stream.Read(c, length);
  c[length] = '\0';
  return c;
}

char* x801::game::readStringFromBitstream16(RakNet::BitStream& stream) {
  uint16_t length;
  stream.Read(length);
  char* c = new char[length + 1];
  stream.Read(c, length);
  c[length] = '\0';
  return c;
}