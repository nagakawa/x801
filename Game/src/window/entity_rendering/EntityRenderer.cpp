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
        MeshEntry entry = { l.x, l.y, (uint16_t) tid };
        mesh.push_back(entry);
      });
    }
    static const char* VERTEX_SOURCE =
      "#version 330 core \n"
      // The position of the vertex, in 16-metre units,
      // from the centre of the northwestmost block of the chunk.
      "layout (location = 0) in vec2 rpos; \n"
      "layout (location = 1) in uint W; \n"
      "layout (location = 2) in vec2 epos; \n"
      "out vec2 TexCoord; \n"
      "flat out uint w; \n"
      // The model-view-projection matrix.
      "uniform mat4 mvp; \n"
      "void main() { \n"
      "  vec3 basePosition = vec3(epos + rpos - 0.5, 0); \n"
      "  gl_Position = mvp * vec4(basePosition, 1.0f); \n"
      "  TexCoord = rpos; \n"
      "  w = W; \n"
      "} \n"
      ;
    static const char* FRAGMENT_SOURCE =
      "#version 330 core \n"
      "in vec2 TexCoord; \n"
      "flat in uint w; \n"
      "out vec4 colour; \n"
      "uniform sampler2D tex; \n"
      "#define DIVISOR 128u \n" // 4096 / 32
      "void main() { \n"
      "  vec2 uvstart = vec2(w % DIVISOR, w / DIVISOR); \n"
      // or (w / DIVISOR) % DIVISOR
      // (if we need to use multiple textures)
      "  vec2 local = mod(TexCoord, 1); \n"
      "  vec2 realtc = (local + uvstart) / DIVISOR; \n"
      // "  colour = vec4(vec3(w % 4u, (w / 4u) % 4u, (w / 16u) % 4u) / 3.0, 1); \n"
      "  colour = texture(tex, realtc); \n"
      "} \n"
      ;
    void EntityBuffer::setUpRender() {
      agl::Shader* vertexShader = new agl::Shader(VERTEX_SOURCE, GL_VERTEX_SHADER);
      agl::Shader* fragmentShader = new agl::Shader(FRAGMENT_SOURCE, GL_FRAGMENT_SHADER);
      program.attach(*vertexShader);
      program.attach(*fragmentShader);
      program.link();
      program.use();
      delete vertexShader;
      delete fragmentShader;
      vao.setActive();
      vbo.feedData(sizeof(squareCoords), squareCoords, GL_STATIC_DRAW);
      // Set vertex coordinates of rect
      glVertexAttribPointer(0,
        2, GL_FLOAT, false,
        2 * sizeof(float), (void*) 0);
      glEnableVertexAttribArray(0);
      ivbo.feedData(mesh.size() * sizeof(MeshEntry), mesh.data(), GL_STATIC_DRAW);
      glVertexAttribIPointer(
        1,
        1, GL_UNSIGNED_SHORT,
        sizeof(MeshEntry), (void*) offsetof(MeshEntry, texID));
      glVertexAttribPointer(
        2,
        2, GL_FLOAT, false
        sizeof(MeshEntry), (void*) offsetof(MeshEntry, x));
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glVertexAttribDivisor(1, 1);
      glVertexAttribDivisor(2, 1);
      program.use();
      tr->texb->bindTo(0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      SETUNSP(program, 1i, "texb", 0);
      tr->texd->bindTo(1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      SETUNSP(program, 1i, "texd", 1);
      glm::vec3 offset(
        xyz.x * 16.0f,
        xyz.y * 16.0f,
        xyz.z
      );
      SETUNSPV(program, 3fv, "offset", glm::value_ptr(offset));
#ifndef NDEBUG
      isset = true;
#endif
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