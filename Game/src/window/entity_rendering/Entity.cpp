#include "window/entity_rendering/Entity.h"

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

namespace x801 {
  namespace game {
    void Entity::updateComponentOfPart(
        size_t partID, size_t componentID,
        std::vector<bool>& isUpdated
    ) {
      // No need to update again
      if (isUpdated[componentID]) return;
      std::vector<glm::vec3>& positions =
        absoluteComponentPositions[partID];
      std::vector<glm::quat>& orientations =
        absoluteComponentOrientations[partID];
      x801::map::Part& part = *(usedParts[partID]);
      x801::map::Component& comp = part.components[componentID];
      size_t parentID = comp.parent;
      // Update the component's parent first
      if (parentID != -1U) {
        updateComponentOfPart(partID, parentID, isUpdated);
      } else {
        // The parent is ground, which has no offset or rotation.
        positions[componentID] = comp.offsetCoordinates;
        orientations[componentID] = comp.offsetAngle;
        return;
      }
      // The absolute position is the position of the parent
      // plus the component offset adjusted for its orientation.
      glm::vec3 absoluteOffset =
        orientations[parentID] * comp.offsetCoordinates;
      positions[componentID] = positions[parentID] + absoluteOffset;
      // The absolute orientation is the orientation of a parent
      // influenced by control angles.
      std::vector<std::string>& controlNames =
        part.controlAnglesByComponent[componentID];
      glm::quat absoluteOrientation = orientations[parentID];
      for (const std::string& name : controlNames) {
        auto it = controlAngles.find(name);
        if (it == controlAngles.end()) continue;
        absoluteOrientation = it->second * absoluteOrientation;
      }
      orientations[componentID] = absoluteOrientation;
    } // What a breather!
    // Update all of the components of a part
    void Entity::updateComponentsOfPart(size_t partID) {
      x801::map::Part& part = *(usedParts[partID]);
      std::vector<bool> isUpdated(part.components.size(), false);
      for (size_t i = 0; i < part.components.size(); ++i) {
        updateComponentOfPart(partID, i, isUpdated);
      }
    }
    void Entity::updatePartTraits(
        size_t partID, std::vector<bool>& isUpdated) {
      if (isUpdated[partID]) return;
      const PartLink& link = links[partID];
      size_t parentID = link.parentPart;
      if (parentID != -1U) {
        updatePartTraits(parentID, isUpdated);
      } else { // No parent
        absolutePartPositions[partID] = link.offset;
        absolutePartOrientations[partID] = link.orientation;
      }
      // (Somewhat) copied from updateComponentOfPart
      // Set position
      glm::vec3 absoluteOffset =
        absolutePartOrientations[parentID] *
        (absoluteComponentPositions[parentID][link.component] + link.offset);
      absolutePartPositions[partID] =
        absolutePartPositions[parentID] + absoluteOffset;
      // Set rotation
      absolutePartOrientations[partID] =
        absoluteComponentOrientations[parentID][link.component] *
        link.orientation *
        absolutePartOrientations[parentID];
    }
    void Entity::updatePartTraits() {
      std::vector<bool> isUpdated(usedParts.size(), false);
      for (size_t i = 0; i < usedParts.size(); ++i) {
        updatePartTraits(i, isUpdated);
      }
    }
    void Entity::update() {
      // Update part positions and orientations
      for (size_t i = 0; i < usedParts.size(); ++i)
        updateComponentsOfPart(i);
      updatePartTraits();
    }
  }
}
