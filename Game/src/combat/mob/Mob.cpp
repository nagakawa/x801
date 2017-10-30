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

#include <movement_constants.h>

namespace x801 {
  namespace game {
    void Mob::advanceFrame(float s, const x801::map::Path& path) {
      using namespace x801::map;
      size_t pathno = (size_t) progress;
      if (pathno >= path.vertices.size() - 1) return;
      float pathLength = path.lengthOfPart(pathno);
      progress += s * MOB_SPEED / pathLength;
      pos = path.progressToCoordinates(progress);
    }
  }
}