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

#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Texture.h>

#include <EntityModel.h>

namespace x801 {
  namespace game {
    struct PartLink {
      size_t parentPart, component;
      glm::vec3 offset;
      glm::quat orientation;
    };
    class Entity {
      /* REFERENCE:
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
      */
    public:
      /*
        Invariants:
        usedParts.size() == usedTextures.size()
        usedParts.size() == usedLinks.size()
        usedParts.size() == absolutePartPositions.size()
        usedParts.size() == absolutePartOrientations.size()
        usedParts.size() == absoluteComponentPositions.size()
        usedParts.size() == absoluteComponentOrientations.size()
        for all i in [0, usedParts.size()):
          absoluteComponentPositions[i].size() ==
            usedParts[i]->components.size()
          absoluteComponentOrientations[i].size() ==
            usedParts[i]->components.size()
      */
      std::vector<x801::map::Part*> usedParts;
      std::vector<agl::Texture*> usedTextures;
      std::unordered_map<std::string, glm::quat> controlAngles;
      std::vector<PartLink> links;
      void update();
    private:
      std::vector<glm::vec3> absolutePartPositions;
      std::vector<glm::quat> absolutePartOrientations;
      // Not quite absolute: these are relative to their grounds
      std::vector<std::vector<glm::vec3>> absoluteComponentPositions;
      std::vector<std::vector<glm::quat>> absoluteComponentOrientations;
      void updatePartTraits();
      void updatePartTraits(size_t partID, std::vector<bool>& isUpdated);
      // Update the components of a part
      void updateComponentsOfPart(
        size_t partID);
      void updateComponentOfPart(
        size_t partID, size_t componentID,
        std::vector<bool>& isUpdated
      );
    };
  }
}
