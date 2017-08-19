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

#include <boost/optional.hpp>

#include <utils.h>

namespace x801 {
  namespace game {
    EntityRenderer::EntityRenderer(ClientWindow* cw, agl::FBOTexMS& ft) {
      this->cw = cw;
      c = cw->c;
      p = c->patcher;
      tv = c->textureView;
      gs = &(c->g);
      // No prefetching a texture, as there might be multiple texture files
      bv = c->blueprintView;
      pv = c->partView;
      assert(
        cw != nullptr && c != nullptr &&
        p != nullptr && tv != nullptr &&
        gs != nullptr && bv != nullptr &&
        tv != nullptr);
      fboMS = ft.ms.fbo;
    }
    void EntityRenderer::setUpRender() {
      a.setUpRender();
#ifndef NDEBUG
      issetup = true;
#endif
    }
    bool EntityRenderer::addTexture(const std::string& id) {
      if (a.locations.count(id) != 0) {
        XTRACE("Texture ", id, " is already in the atlas");
        return true;
      }
      boost::optional<agl::Texture> tex =
        tv->getTextureTransient("textures/entity" + id + ".png");
      if (!tex) {
        XTRACE("Texture ", id, " does not exist");
        return false;
      }
      a.insert(id, *tex);
      XTRACE("Texture ", id, " inserted into atlas");
      return true;
    }
    uint32_t EntityRenderer::addEntity(Entity&& e) {
      entities[nextID] = std::move(e);
      return nextID++;
    }
    uint32_t EntityRenderer::addEntity(const std::string& name) {
      Entity e(*pv, *(bv->getBlueprint(name)));
      return addEntity(std::move(e));
    }
  }
}