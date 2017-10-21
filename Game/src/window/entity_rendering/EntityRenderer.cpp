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

#include <imgui.h>

#include <utils.h>

#include "x801_rect.h"
#include "Client.h"
#include "window/ClientWindow.h"
#include "window/Patcher.h"
#include "window/patcher_views/TextureView.h"
#include "window/entity_rendering/EntityManager.h"

namespace x801 {
  namespace game {
    void EntityBuffer::push() {
      vao.setActive();
      ivbo.setActive();
      ivbo.feedData(mesh.size() * sizeof(MeshEntry), mesh.data(), GL_STATIC_DRAW);
      glVertexAttribIPointer(
        1,
        1, GL_UNSIGNED_SHORT,
        sizeof(MeshEntry), (void*) offsetof(MeshEntry, texID));
      glVertexAttribPointer(
        2,
        2, GL_FLOAT, false,
        sizeof(MeshEntry), (void*) offsetof(MeshEntry, x));
    }
    void EntityBuffer::switchToFull() {
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
      ImGui::SetNextWindowSize(
        ImVec2((float) er->cw->getWidth(), (float) er->cw->getHeight()));
      ImGui::Begin(
        "##fullscn", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs
      );
    }
    void EntityBuffer::returnFromFull() {
      ImGui::End();
      ImGui::PopStyleColor();
    }
    void drawTextCentreBottom(
        ImDrawList* drawList,
        const char* s,
        const ImVec2& pos,
        const glm::vec4& colour) {
      ImVec2 size = ImGui::CalcTextSize(s);
      const ImVec2& tlpos = ImVec2(pos.x - size.x / 2, pos.y - size.y);
      uint32_t col32 = ImGui::GetColorU32(
        ImVec4(colour.r, colour.g, colour.b, colour.a)
      );
      drawList->AddRectFilled(
        ImVec2(tlpos.x - 3, tlpos.y - 3),
        ImVec2(tlpos.x + size.x + 3, tlpos.y + size.y + 3),
        0x80000000, 1
      );
      drawList->AddText(tlpos, col32, s);
    }
    glm::vec2 EntityBuffer::getBottom(float x, float y) {
      float width = (float) er->cw->getWidth();
      float height = (float) er->cw->getHeight();
      glm::vec4 bottom = er->cw->mvp * glm::vec4(x, y, 0, 1);
      return glm::vec2(
        width * (0.5 * bottom.x + 0.5),
        height * (0.5 * bottom.y + 0.5)
      );
    }
    void EntityBuffer::feed() {
      EntityManager* em = er->em;
      mesh.clear();
      switchToFull();
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      // XXX this iterates over every entity.
      // This could be problematic for large numbers of entities.
      em->forEachOver([this, drawList](Entity& e, OverheadName& name) {
        Location l = e.getLocation();
        size_t tid = e.getTexture();
        if (l.z != er->gs->selfPosition.z) return;
        MeshEntry entry = { l.x, l.y, (uint16_t) tid };
        mesh.push_back(entry);
        if (name.classifier == EntityClassifier::NONE) return;
        glm::vec2 bottom = getBottom(l.x, l.y - 0.5);
        drawTextCentreBottom(
          drawList,
          name.format().c_str(),
          ImVec2(bottom.x, bottom.y),
          name.colour());
      });
      returnFromFull();
      /*
        XXX: for some reason, this check must occur here and not in
        push(), although the behaviour should be equivalent.
        [as of g++ (GCC) 8.0.0 20170501 (experimental)]
        Undefined behaviour, perhaps?
      */
      if (mesh.size() == 0) return;
      push();
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
      "  if (w == 65535u) colour = vec4(1.0, 0.0, 0.0, 1.0); \n"
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
      push();
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glVertexAttribDivisor(1, 1);
      glVertexAttribDivisor(2, 1);
      program.use();
      er->tex->bindTo(0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      SETUNSP(program, 1i, "texb", 0);
#ifndef NDEBUG
      setup = true;
#endif
    }
    void EntityBuffer::render() {
#ifndef NDEBUG
      if (!setup)
        throw "ChunkBuffer: render() called before setUpRender()";
#endif
      if (mesh.size() == 0) return;
      for (const auto& m : mesh) {
        std::cerr << "(" << m.x << ", " << m.y << ", " << m.texID << ") ";
      }
      std::cerr << '\n';
      er->tex->bindTo(0);
      glDisable(GL_DEPTH_TEST);
      glDepthMask(false);
      glEnable(GL_BLEND);
      vao.setActive();
      program.use();
      glm::mat4 mvp = er->cw->mvp;
      SETUNSPM(program, 4fv, "mvp", glm::value_ptr(mvp));
      glDrawArraysInstanced(GL_TRIANGLES, 0, 6, mesh.size());
      glDepthMask(true);
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
      buffer = new EntityBuffer(this);
      std::stringstream eFile =
        c->patcher->getSStream("textures/entity/entities.tti");
      if (Entity::tb == nullptr) {
        tb = new x801::map::EntityTextureBindings(eFile);
        Entity::tb = tb;
      }
    }
  }
}