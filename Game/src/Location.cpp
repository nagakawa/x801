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

// TODO: support passing a reference to a map and use collision detection.
bool x801::game::Location::applyKeyInput(KeyInput input, RakNet::Time last) {
  uint32_t inputs = input.inputs;
  if (inputs == 0) return true;
  float delta = ((ssize_t) input.time - (ssize_t) last) / 1000.0f;
  int isMovingUp = (inputs & (1 << K_OFFSET_FORWARD)) != 0;
  int isMovingDown = (inputs & (1 << K_OFFSET_BACK)) != 0;
  int isMovingLeft = (inputs & (1 << K_OFFSET_LEFT)) != 0;
  int isMovingRight = (inputs & (1 << K_OFFSET_RIGHT)) != 0;
  float advancex = PLAYER_SPEED * delta * (isMovingRight - isMovingLeft);
  float advancey = PLAYER_SPEED * delta * (isMovingDown - isMovingUp);
  x += advancex;
  y += advancey;
  rot =
    isMovingRight ? 0 :
    isMovingUp ? 1 :
    isMovingLeft ? 2 :
    isMovingDown ? 3 :
    rot;
  return true;
}
