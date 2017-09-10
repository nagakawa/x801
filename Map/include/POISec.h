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

#include <stddef.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <utils.h>

namespace x801 {
  namespace map {
    class POISec {
    public:
      class POI {
      public:
        int16_t x, y;
        int8_t z;
      };
      class EntityPOI {
      public:
        EntityPOI(
            std::string texname, size_t offset,
            std::string title, std::string name) :
          texname(texname), offset(offset),
          title(title), name(name) {}
        std::string texname;
        size_t offset;
        std::string title, name;
      };
      POISec() : present(false) {}
      POISec(std::istream& fh);
      void write(std::ostream& fh) const;
      bool present;
      std::vector<POI> pois;
      std::unordered_map<size_t, EntityPOI> entityPOIs;
    };
  }
}