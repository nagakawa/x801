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
#include <string.h>
#include <functional>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <utils.h>
#include <Version.h>

namespace x801 {
  namespace map {
    class Component {
    public:
      Component(std::istream& fh);
      size_t parent;
      glm::quat offsetAngle;
      glm::vec3 offsetCoordinates;
      glm::vec3 axisScale;
    }
    class Part {
    public:
      Part(std::istream& fh);
    private:
      glm::vec3 hitboxSize;
      //
    }
  }
}