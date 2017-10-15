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

#include <EBO.h>
#include <FBO.h>
#include <Shader.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <VAO.h>
#include <VBO.h>

#include <Area.h>
#include <Chunk.h>

namespace x801 {
  namespace game {
    class ClientWindow;
    class EntityRenderer;
  }
}
#include "Client.h"
#include "GameState.h"
#include "window/ClientWindow.h"
#include "window/Patcher.h"
#include "window/patcher_views/BlueprintView.h"
#include "window/patcher_views/PartView.h"
#include "window/patcher_views/TextureView.h"
#include "window/entity_rendering/Entity.h"
#include "window/entity_rendering/EntityManager.h"

namespace x801 {
  namespace game {
    class EntityBuffer {
    public:
      struct MeshEntry {
        float x, y;
        uint16_t texID;
      };
      EntityBuffer(EntityRenderer* er) :
        er(er) {};
      void feed();
      void setUpRender();
      void render();
      void renderOverheads();
      EntityRenderer* er;
    private:
      void push();
      void switchToFull();
      void returnFromFull();
      glm::vec2 getBottom(float x, float y);
      std::vector<MeshEntry> mesh;
      agl::VAO vao;
      agl::VBO vbo;
      agl::VBO ivbo;
      agl::ShaderProgram program;
#ifndef NDEBUG
      bool setup;
#endif
    };
    class EntityRenderer {
    public:
      EntityRenderer(ClientWindow* cw, agl::FBOTexMS& ft, EntityManager* em);
      ~EntityRenderer() {
        delete buffer;
        delete tb;
      }
      void draw();
      // ChunkBuffer* summon(const x801::map::ChunkXYZ& pos);
      ClientWindow* cw;
      Client* c;
      Patcher* p;
      TextureView* tv;
      ClientGameState* gs;
      agl::Texture* tex;
      EntityManager* em;
      std::shared_ptr<agl::FBO> fboMS;
      void setUpRender() { buffer->setUpRender(); }
      void feed() { buffer->feed(); }
      void render() { buffer->render(); }
    private:
      EntityBuffer* buffer = nullptr;
      x801::map::EntityTextureBindings* tb;
    };
  }
}