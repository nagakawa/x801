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
#include <array>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <boost/functional/hash.hpp>
#include "portable_endian.h"

// Since I expect Int and UInt to be used frequently,
// I'm placing it in the default namespace.

typedef int_fast32_t Int;
typedef uint_fast32_t UInt;

#define DEFINE_OFFSETS_AND_FLAGS(type, name, offsetValue) \
  const type OFFSET#name = offsetValue; \
  const type FLAG#name = 1 << offsetValue;

namespace x801 {
  namespace base {
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
    template<> uint16_t convLER<uint16_t>(uint16_t x);
    template<> uint32_t convLER<uint32_t>(uint32_t x);
    template<> uint64_t convLER<uint64_t>(uint64_t x);
    template<> uint16_t convLEW<uint16_t>(uint16_t x);
    template<> uint32_t convLEW<uint32_t>(uint32_t x);
    template<> uint64_t convLEW<uint64_t>(uint64_t x);
    template<> int16_t convLER<int16_t>(int16_t x);
    template<> int32_t convLER<int32_t>(int32_t x);
    template<> int64_t convLER<int64_t>(int64_t x);
    template<> int16_t convLEW<int16_t>(int16_t x);
    template<> int32_t convLEW<int32_t>(int32_t x);
    template<> int64_t convLEW<int64_t>(int64_t x);
    template<typename T> T readInt(std::istream& fh) {
      T val;
      fh.read(reinterpret_cast<char*> (&val), sizeof(T));
      return convLER(val);
    }
    template<typename T> void writeInt(std::ostream& fh, T val) {
      val = convLEW(val);
      fh.write(reinterpret_cast<char*> (&val), sizeof(T));
    }
    std::stringstream fromCharArray(char* array, unsigned int size);
    template<int len> std::string construct(
        const char (&s)[len],
        bool addNull = false) {
      return std::string{s, static_cast<size_t>(len - (addNull ? 0 : 1))};
    }
    const UInt CHUNK = 131072L;
    int readZipped(
        std::istream& f,
        char*& block,
        uint32_t& amtReadC,
        uint32_t& amtReadU
    );
    int writeZipped(
        std::ostream& f,
        const char* block,
        uint32_t len,
        uint32_t& amtWrittenC
    );
    void writeRandomBytes(uint8_t* buffer, int length);
    template<typename T, size_t n>
    struct STDArrayHash {
      size_t operator()(const std::array<T, n>& array) const {
        return boost::hash_value(array);
      }
    };

    template <typename Str>
    size_t getLength(const Str s) {
      static_assert(x801::base::assert_false<Str>::value,
        "getLength has not been specialised for this type");
      (void) s;
      return 0;
    }
    template <typename Str>
    const char* getPointer(const Str s) {
      static_assert(x801::base::assert_false<Str>::value,
        "getPointer has not been specialised for this type");
      (void) s;
      return nullptr;
    }
    template<>
    size_t getLength(const char* s);
    template<>
    size_t getLength(std::string s);
    template<>
    const char* getPointer(const char* s);
    template<>
    const char* getPointer(std::string s);

    std::string slurp(std::ifstream& fh);

    // Simple way to get the path of the current exe.
    // Doesn't work on ALL platforms, but it covers the big 3
    std::string getPathOfCurrentExecutable();

    bool canBeConvertedToPositiveInt(const char* s, int* out = nullptr);
  }
}
