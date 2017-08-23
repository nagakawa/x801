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

#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>

#include <EBO.h>
#include <FBO.h>
#include <Shader.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <VAO.h>
#include <VBO.h>

#include <Area.h>
#include <Chunk.h>
#include <EntityModel.h>

namespace x801 {
  namespace game {
    class ClientWindow;
    class EntityRenderer;
  }
}
#include "Client.h"
#include "GameState.h"
#include "window/Axes.h"
#include "window/ClientWindow.h"
#include "window/Patcher.h"
#include "window/patcher_views/BlueprintView.h"
#include "window/patcher_views/PartView.h"
#include "window/patcher_views/TextureView.h"
#include "window/entity_rendering/Atlas.h"
#include "window/entity_rendering/Entity.h"

namespace x801 {
  namespace game {
    // Ranges of vacancies.
    // key = start, value = size (i. e. end - start)
    // (half-open interval)
    // Start locations are unique; sizes might repeat.
    using VacancyList = boost::bimap<
        boost::bimaps::set_of<size_t>,
        boost::bimaps::multiset_of<size_t>>;
    struct EMVertex {
      float x, y, z;
      float u, v;
    };
    struct EMLocation {
      size_t pageno; // Page number of part
      size_t start; // Start index of part
      size_t count; // How many vertices this part takes
      std::vector<EMVertex> vertices; // The vertices
    };
    static_assert(
      offsetof(EMVertex, x) + 4 == offsetof(EMVertex, y)
        && offsetof(EMVertex, y) + 4 == offsetof(EMVertex, z)
        && offsetof(EMVertex, u) + 4 == offsetof(EMVertex, v),
      "Basic offset checks");
    class EntityMeshBuffer {
    public:
      EntityMeshBuffer(EntityRenderer* er, size_t i) :
          er(er), i(i) {}
      void createMesh();
      void setUpRender();
      void render();
    private:
      std::vector<EMVertex> vertices;
      EntityRenderer* er;
      agl::VBO vbo;
      agl::VAO vao;
      agl::ShaderProgram program;
      VacancyList vacancies;
      size_t allocate(size_t size);
      void free(size_t start, size_t size);
      size_t i;
#ifndef NDEBUG
      bool setup = false;
#endif
      friend class EntityRenderer;
    };
    class EntityRenderer {
    public:
      EntityRenderer(ClientWindow* cw, agl::FBOTexMS& ft);
      void setUpRender();
      void render();
      // Currently not threadsafe (race condition on nextID).
      // May make these methods so if circumstances demand it.
      uint32_t addEntity(Entity&& e);
      uint32_t addEntity(const std::string& name);
      ClientWindow* cw;
      Client* c;
      Patcher* p;
      TextureView* tv;
      BlueprintView* bv;
      PartView* pv;
      ClientGameState* gs;
      std::shared_ptr<agl::FBO> fboMS;
      Atlas a;
    private:
      /*std::unordered_map<
          x801::map::ChunkXYZ,
          ChunkMeshBuffer,
          x801::map::ChunkHasher> cmbs;*/
      std::unordered_map<uint32_t, Entity> entities;
      std::unordered_map<uint32_t, EMLocation> locations;
      uint32_t nextID = 0;
      std::vector<EntityMeshBuffer> embs;
      bool addTexture(const std::string& id);
#ifndef NDEBUG
      bool issetup = false;
#endif
    };
  }
}