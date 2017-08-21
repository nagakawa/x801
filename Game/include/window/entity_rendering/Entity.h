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

#include "window/patcher_views/PartView.h"
#include "window/patcher_views/TextureView.h"

namespace x801 {
  namespace game {
    struct PartLink {
      size_t parentPart, component;
      glm::vec3 offset;
      glm::quat orientation;
    };
    class Entity {
    public:
      Entity() {}
      Entity(
        PartView& pv,
        const x801::map::Blueprint& bp
      );

#define XAPP \
  X(usedParts), \
  X(usedTextures), \
  X(controlAngles), \
  X(links), \
  X(indicesByID), \
  X(absolutePartPositions), \
  X(absolutePartOrientations), \
  X(absoluteComponentPositions), \
  X(absoluteComponentOrientations)
#define X(n) n(std::move(e.n))

      Entity(Entity&& e) :
        XAPP {}

#undef X
#define X(n) n = std::move(e.n)

      Entity& operator=(Entity&& e) {
        XAPP;
        return *this;
      }

#undef X
#undef XAPP

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
      bool update();
      bool isDirty() { return dirty; }
      void addPart(
          const x801::map::Part* part,
          const std::string& tex,
          const PartLink& link,
          const std::string& partID) {
        size_t i = usedParts.size();
        indicesByID[partID] = i;
        usedParts.push_back(part);
        usedTextures.push_back(tex);
        links.push_back(link);
        absolutePartPositions.emplace_back();
        absolutePartOrientations.emplace_back();
        size_t nComponents = part->components.size();
        absoluteComponentPositions.emplace_back(nComponents);
        absoluteComponentOrientations.emplace_back(nComponents);
        dirty = true;
      }
      void setControlAngle(const std::string& name, glm::quat angle) {
        controlAngles[name] = angle;
        dirty = true;
      }
    private:
      bool dirty = false;
      std::vector<const x801::map::Part*> usedParts;
      std::vector<std::string> usedTextures;
      std::unordered_map<std::string, glm::quat> controlAngles;
      std::vector<PartLink> links;
      std::unordered_map<std::string, size_t> indicesByID;
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
