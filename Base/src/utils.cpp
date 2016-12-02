#include "utils.h"

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

#include <sstream>
#include <string>

using namespace x801::base;

template<> uint16_t x801::base::convLER<uint16_t>(uint16_t x) { return le16toh(x); }
template<> uint32_t x801::base::convLER<uint32_t>(uint32_t x) { return le32toh(x); }
template<> uint64_t x801::base::convLER<uint64_t>(uint64_t x) { return le64toh(x); }
template<> uint16_t x801::base::convLEW<uint16_t>(uint16_t x) { return htole16(x); }
template<> uint32_t x801::base::convLEW<uint32_t>(uint32_t x) { return htole32(x); }
template<> uint64_t x801::base::convLEW<uint64_t>(uint64_t x) { return htole64(x); }
template<> int16_t x801::base::convLER<int16_t>(int16_t x) { return le16toh(x); }
template<> int32_t x801::base::convLER<int32_t>(int32_t x) { return le32toh(x); }
template<> int64_t x801::base::convLER<int64_t>(int64_t x) { return le64toh(x); }
template<> int16_t x801::base::convLEW<int16_t>(int16_t x) { return htole16(x); }
template<> int32_t x801::base::convLEW<int32_t>(int32_t x) { return htole32(x); }
template<> int64_t x801::base::convLEW<int64_t>(int64_t x) { return htole64(x); }

std::stringstream x801::base::fromCharArray(char* array, unsigned int size) {
  std::string s{array, size};
  return std::stringstream(s);
}
