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

#include <iostream>
#include <stdint.h>

namespace x801 {
  namespace base {
    const int ALPHA = 0;
    const int BETA = 1;
    const int RC = 2;
    const int RELEASE = 3;
    struct Version {
      uint16_t vMajor, vMinor, vPatch; // ouli
      uint16_t prerelease;
      Version(uint16_t x, uint16_t y, uint16_t z = 0, uint16_t pre = 0xC000) :
        vMajor(x), vMinor(y), vPatch(z), prerelease(pre) {}
      Version(
          uint16_t x, uint16_t y, uint16_t z,
          uint16_t letter, uint16_t preNum = 0) :
        vMajor(x), vMinor(y), vPatch(z), prerelease((letter << 14) | preNum) {}
      Version(std::istream& fh);
      int getPrereleaseType() { return prerelease >> 14; }
      int getPrereleaseNumber() { return prerelease & 0x3FFF; }
      bool operator==(Version& other);
      bool operator<(Version& other);
      bool canSucceed(Version& other);
      void write(std::ostream& fh);
    };
  }
}
