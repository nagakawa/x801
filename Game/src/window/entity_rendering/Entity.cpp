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

#include "window/ClientWindow.h"

namespace x801 {
  namespace game {
    x801::map::EntityTextureBindings* Entity::tb = nullptr;
    ClientWindow* PlayerEntity::cw = nullptr;
    static size_t playerTexID = -1;
    size_t PlayerEntity::getTexture() {
      if (playerTexID + 1 == 0) {
        // Get player tex ID
        playerTexID = tb->getTexID("placeholder");
        if (playerTexID + 1 == 0) {
          std::cerr << "playerTexID == -1U\n";
          return -1U;
        }
      }
      return playerTexID + l.rot + ((walkFrame / 15) % 2) * 4;
    }
    OverheadName PlayerEntity::overheadName() {
      return OverheadName(
        "",
        cw->c->getUsername(id),
        0, EntityClassifier::PLAYER
      );
    }
    OverheadName NPCEntity::overheadName() {
      return OverheadName(
        title,
        name,
        0, EntityClassifier::NPC
      );
    }
    OverheadName MobEntity::overheadName() {
      return OverheadName(
        mi->title,
        mi->dispname,
        0, (EntityClassifier) mi->type
      );
    }
  }
}