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
    size_t EntityMeshBuffer::allocate(size_t size) {
      auto end = vacancies.right.end();
      // Search for the smallest vacancy that will hold a block this big
      auto it = vacancies.right.lower_bound(size);
      if (it == end) {
        // No such vacancy; make room
        size_t oldSize = vertices.size();
        vertices.resize(oldSize + size);
        return oldSize;
      }
      // There is a vacancy.
      size_t binSize = it->first;
      size_t binStart = it->second;
      if (binSize == size) {
        // No extra room
        vacancies.right.erase(it);
        return binStart;
      }
      // There is extra room
      size_t newBinSize = binSize - size;
      size_t newBinStart = binStart + size;
      vacancies.right.erase(it);
      vacancies.left.insert(
        std::pair<size_t, size_t>(newBinStart, newBinSize));
      return binStart;
    }
    void EntityMeshBuffer::free(size_t start, size_t size) {
      // Determine where to coalesce
      bool shouldCoalesceLeft = false;
      auto ileft = vacancies.left.lower_bound(start);
      auto iright = ileft;
      assert(iright->first > start);
      if (ileft != vacancies.left.begin()) {
        --ileft; // Now ileft is at the last key less than start
        if (ileft->first + ileft->second == start)
          shouldCoalesceLeft = true;
      }
      bool shouldCoalesceRight = start + size == iright->first;
      // Determine the start point and the size of the new free block
      size_t newStart = start;
      size_t newSize = size;
      if (shouldCoalesceLeft) {
        newStart = ileft->first;
        newSize += ileft->second;
      }
      if (shouldCoalesceRight) {
        // newStart is unchanged
        newSize += iright->second;
      }
      if (shouldCoalesceLeft)
        iright = vacancies.left.erase(ileft);
      if (shouldCoalesceRight)
        vacancies.left.erase(iright);
      vacancies.left.insert(
        std::pair<size_t, size_t>(newStart, newSize));
    }
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