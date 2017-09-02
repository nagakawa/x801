#include "Location.h"

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

using namespace x801::game;

#include <math.h>
#include <iostream>

#include "movement_constants.h"

namespace x801 {
  namespace game {
    bool Location::applyKeyInput(KeyInput input, RakNet::Time last) {
      uint32_t inputs = input.inputs;
      if (inputs == 0) return true;
      float delta = ((ssize_t) input.time - (ssize_t) last) / 1000.0f;
      int isMovingUp = (inputs & (1 << K_OFFSET_FORWARD)) != 0;
      int isMovingDown = (inputs & (1 << K_OFFSET_BACK)) != 0;
      int isMovingLeft = (inputs & (1 << K_OFFSET_LEFT)) != 0;
      int isMovingRight = (inputs & (1 << K_OFFSET_RIGHT)) != 0;
      x += PLAYER_SPEED * delta * (isMovingRight - isMovingLeft);
      y += PLAYER_SPEED * delta * (isMovingDown - isMovingUp);
      rot =
        isMovingRight ? 0 :
        isMovingUp ? 1 :
        isMovingLeft ? 2 :
        isMovingDown ? 3 :
        rot;
      return true;
    }
    // This is the centre-to-edge distance of a hitbox.
    // An entity's hitbox is a square.
    static const float HITBOX_SIZE = 0.3f;
    bool Location::isJammed(const x801::map::Area& a) {
      using namespace x801::map;
      TileSec& ts = a.getTileSec();
      // Bounds for which blocks need to be collision-checked
      int xmin = (int) (x - 1 - HITBOX_SIZE);
      int ymin = (int) (y - 1 - HITBOX_SIZE);
      int xmax = (int) (x + HITBOX_SIZE);
      int ymax = (int) (y + HITBOX_SIZE);
      // Check the necessary blocks
      for (int xc = xmin; xc <= xmax; ++xc) {
        for (int yc = ymin; yc <= ymax; ++yc) {
          BlockXYZ xyz(xc, yc, z);
          Block b = ts.getBlock(xyz);
          if (b.isSolid() || b.getBaseID() == 0)
            return true;
        }
      }
      return false;
    }
    bool Location::applyKeyInput(
        KeyInput input,
        RakNet::Time last,
        const x801::map::Area& a) {
      // No need to check if you're not moving
      if (input.inputs == 0) return true;
      // Unconditionally allow movement when
      // already jammed
      if (isJammed(a)) return applyKeyInput(input, last);
      // Save old position
      Location old = *this;
      applyKeyInput(input, last);
      if (isJammed(a)) {
        *this = old;
        return false;
      }
      return true;
    }
  }
}
