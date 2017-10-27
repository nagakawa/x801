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
#include <iosfwd>
#include <string>
#include <glm/glm.hpp>

#include "combat/Stats.h"

namespace x801 {
  namespace game {
    class MobInfo {
    public:
      MobInfo(std::istream& fh);
      Stats stats;
      std::string id;
      std::string dispname, title;
      uint16_t rank, type;
      std::string texname;
    };
  }
}