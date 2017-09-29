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

#include <glm/glm.hpp>

namespace x801 {
  namespace map { class Path; }
  namespace game {
    class MobInfo;
    class Mob {
    public:
      bool marked = false;
      glm::vec2 pos;
      float progress;
      MobInfo* info;
      void advanceFrame(float s, const x801::map::Path& path);
    };
    struct MobGetXY {
      glm::vec2 getPos(const Mob& m) const { return m.pos; }
    };
  }
}