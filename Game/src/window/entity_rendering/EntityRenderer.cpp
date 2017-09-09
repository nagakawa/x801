#include "window/entity_rendering/EntityRenderer.h"

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

#include <assert.h>
#include <utility>

#include <boost/optional.hpp>

#include <utils.h>

namespace x801 {
  namespace game {
    void EntityBuffer::feed() {
      EntityManager* em = er->em;
      mesh.clear();
      // XXX this iterates over every entity.
      // This could be problematic for large numbers of entities.
      em->forEach([this](Entity& e) {
        Location l = e.getLocation();
        size_t tid = e.getTexture();
        if (l.z != er->gs->selfPosition.z) return;
        MeshEntry entry = { (uint16_t) tid, l.x, l.y };
        mesh.push_back(entry);
      });
    }
    EntityRenderer::EntityRenderer(ClientWindow* cw, agl::FBOTexMS& ft, EntityManager* em) {
      this->cw = cw;
      c = cw->c;
      p = c->patcher;
      tv = c->textureView;
      gs = &(c->g);
      tex = tv->getTexture("textures/entity/entities.0.png");
      this->em = em;
      assert(
        cw != nullptr && c != nullptr &&
        p != nullptr && tv != nullptr &&
        gs != nullptr && tex != nullptr &&
        em != nullptr);
      fboMS = ft.ms.fbo;
    }
  }
}