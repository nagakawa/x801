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
#include <unordered_map>
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
      void write(std::ostream& fh) const;
      size_t parent;
      glm::quat offsetAngle;
      glm::vec3 offsetCoordinates;
      glm::vec3 axisScale;
    };
    class PFace;
    class PFaceVertex {
    public:
      PFaceVertex(std::istream& fh);
      void write(std::ostream& fh) const;
      size_t cindex;
      glm::vec3 offset;
      glm::vec2 uv;
    private:
      PFaceVertex() {}
      friend class PFace;
    };
    class PFace {
    public:
      PFace(std::istream& fh);
      void write(std::ostream& fh) const;
      PFaceVertex vertices[3];
    };
    class Part {
    public:
      Part(std::istream& fh);
      void write(std::ostream& fh) const;
      glm::vec3 hitboxSize;
      std::vector<Component> components;
      std::vector<PFace> faces;
      std::vector<std::string> componentNames;
      std::unordered_map<std::string, size_t> componentIndicesByName;
      std::unordered_map<std::string, std::vector<size_t>> controlAngles;
      std::unordered_map<size_t, std::vector<std::string>> controlAnglesByComponent;
      std::string name;
    };
  }
}