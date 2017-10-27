#include "combat/mob/Mob.h"

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

#include <math.h>

#include <PathSec.h>

namespace x801 {
  namespace game {
    static constexpr float MOB_SPEED = 2.5f;
    void Mob::advanceFrame(float s, const x801::map::Path& path) {
      using namespace x801::map;
      size_t pathno = (size_t) progress;
      if (pathno >= path.vertices.size() - 1) return;
      const Path::Vertex& curr = path.vertices[pathno];
      const Path::Vertex& next = path.vertices[pathno + 1];
      float pathLength = hypotf(next.x - curr.x, next.y - curr.y);
      progress += s * MOB_SPEED / pathLength;
      if (progress >= path.vertices.size() - 1) {
        pos.x = path.vertices[path.vertices.size() - 1].x;
        pos.y = path.vertices[path.vertices.size() - 1].y;
      } else {
        pathno = (size_t) progress;
        float resid = progress - pathno;
        const Path::Vertex& curr = path.vertices[pathno];
        const Path::Vertex& next = path.vertices[pathno + 1];
        pos.x = next.x * resid + curr.x * (1 - resid);
        pos.y = next.y * resid + curr.y * (1 - resid);
      }
    }
  }
}