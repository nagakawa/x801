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
#include <string>
#include <unordered_map>

namespace x801 {
  namespace map {
    class Atlas {
      struct Elem {
        uint16_t pageno, x1, y1, x2, y2;
        Elem(
            uint16_t pageno,
            uint16_t x1, uint16_t y1,
            uint16_t x2, uint16_t y2) :
          pageno(pageno), x1(x1), y1(y1), x2(x2), y2(y2) {}
        Elem(std::istream& fh);
        void write(std::ostream& fh) const;
      };
    public:
      Atlas(std::istream& fh);
      void write(std::ostream& fh) const;
      std::unordered_map<std::string, Elem> elems;
    };
  }
}