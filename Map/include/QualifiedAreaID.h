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
#include <boost/functional/hash.hpp>

namespace x801 {
  namespace map {
    struct QualifiedAreaID {
      uint16_t worldID;
      uint16_t areaID;
    };
    inline bool operator==(QualifiedAreaID a, QualifiedAreaID b) {
      return a.worldID == b.worldID && a.areaID == b.areaID;
    }
    struct QualifiedAreaIDHash {
      size_t operator()(const QualifiedAreaID& id) const {
        size_t val = 0;
        boost::hash_combine(val, id.worldID);
        boost::hash_combine(val, id.areaID);
        return val;
      }
    };
    struct QualifiedAreaIDEqual {
      bool operator()(QualifiedAreaID a, QualifiedAreaID b) const {
        return a == b;
      }
    };
  }
}