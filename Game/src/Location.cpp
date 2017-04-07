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

// TODO: support passing a reference to a map and use collision detection.
bool x801::game::Location::applyKeyInput(KeyInput input, RakNet::Time last) {
  uint32_t inputs = input.inputs;
  if (inputs == 0) return true;
  float delta = ((ssize_t) input.time - (ssize_t) last) / 1000.0f;
  int isMovingForward = (inputs & (1 << K_OFFSET_FORWARD)) != 0;
  int isMovingBack = (inputs & (1 << K_OFFSET_BACK)) != 0;
  int isTurningLeft = (inputs & (1 << K_OFFSET_LEFT)) != 0;
  int isTurningRight = (inputs & (1 << K_OFFSET_RIGHT)) != 0;
  float advance = PLAYER_SPEED * delta * (isMovingForward - isMovingBack);
  x += advance * cosf(rot);
  y += advance * sinf(rot);
  rot += PLAYER_ANGULAR_VELOCITY * delta * (isTurningLeft - isTurningRight);
  if (rot > 10 || rot < -10) {
    std::cout << "Shit\n";
  }
  return true;
}
