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
    class Path {
    public:
      struct Vertex {
        int16_t x, y;
      };
      Path(std::istream& fh);
      void write(std::ostream& fh) const;
      int z;
      std::vector<Vertex> vertices;
      std::vector<std::string> mobNames;
      std::vector<uint8_t> weights;
      std::vector<std::string> bossNames;
    };
    class PathSec {
    public:
      PathSec() {}
      PathSec(std::istream& fh);
      void write(std::ostream& fh) const;
      std::vector<Path> paths;
      bool present = false;
    };
  }
}