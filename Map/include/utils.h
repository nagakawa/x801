#pragma once

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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error Only C++11 or later supported.
#endif

#include <stdint.h>
#include <iostream>
#include "portable_endian.h"

namespace x801 {
  namespace map {
    // Thanks http://stackoverflow.com/a/33414109/3130218
    template<typename T> struct assert_false : std::false_type {};
    template<typename T> T convLER(T) {
      static_assert(assert_false<T>::value,
        "convLER has not been specialised for this type");
      return 0;
    }
    template<typename T> T convLEW(T) {
      static_assert(assert_false<T>::value,
        "convLEW has not been specialised for this type");
      return 0;
    }
    template<> uint16_t convLER<uint16_t>(uint16_t x) { return le16toh(x); }
    template<> uint32_t convLER<uint32_t>(uint32_t x) { return le32toh(x); }
    template<> uint64_t convLER<uint64_t>(uint64_t x) { return le64toh(x); }
    template<> uint16_t convLEW<uint16_t>(uint16_t x) { return htole16(x); }
    template<> uint32_t convLEW<uint32_t>(uint32_t x) { return htole32(x); }
    template<> uint64_t convLEW<uint64_t>(uint64_t x) { return htole64(x); }
    template<typename T> T readInt(std::istream& fh) {
      T val;
      fh.read(reinterpret_cast<char*> (&val), sizeof(T));
      return convLER(val);
    }
    template<typename T> void writeInt(std::ostream& fh, T val) {
      val = convLEW(val);
      fh.write(reinterpret_cast<char*> (&val), sizeof(T));
    }
  }
}
