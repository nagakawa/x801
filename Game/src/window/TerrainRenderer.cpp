#include "window/TerrainRenderer.h"

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
#include <chrono>
#include <math.h>
#include <sstream>
#include <utility>

#include <imgui.h>

#include <static_block.h>

#include "rect.h"

using namespace x801::game;

x801::game::TerrainRenderer::TerrainRenderer(ClientWindow* cw, agl::FBOTexMS& ft) {
  this->cw = cw;
  c = cw->c;
  p = c->patcher;
  tv = c->textureView;
  gs = &(c->g);
  texb = tv->getTexture("textures/terrain/blocks.0.png");
  texd = tv->getTexture("textures/decorations/blocks.0.png");
  assert(
    cw != nullptr && c != nullptr &&
    p != nullptr && tv != nullptr &&
    gs != nullptr && texb != nullptr
    && texd != nullptr);
  fboMS = ft.ms.fbo;
#ifndef NDEBUG
  axes.setUpRender();
  axes.setScale(100);
#endif
}

x801::map::Chunk* x801::game::TerrainRenderer::getChunk(const x801::map::ChunkXYZ& pos) {
  std::shared_ptr<x801::map::Area> a = gs->currentArea.getArea();
  if (a == nullptr) {
    // Request area
    gs->currentArea.setArea(c->mapView->getArea(gs->selfPosition.areaID));
    if (a == nullptr) return nullptr;
  }
  x801::map::TileSec& ts = a->getTileSec();
  auto it = ts.getChunks().find(pos);
  if (it == ts.getChunks().end()) return nullptr;
  return &(it->second);
}

ChunkBuffer* x801::game::TerrainRenderer::summon(const x801::map::ChunkXYZ& pos) {
  auto it = cmbs.find(pos);
  if (it != cmbs.end()) return &(it->second);
  x801::map::Chunk* chunk = getChunk(pos);
  if (chunk == nullptr) return nullptr;
  cmbs.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(pos),
    std::forward_as_tuple(pos, this, chunk)
  );
  auto it2 = cmbs.find(pos);
  auto res = &(it2->second);
  res->setUpRender();
  return res;
}

static constexpr int RADIUS = 2;

void x801::game::TerrainRenderer::draw() {
  bool isChatWindowOpen = ImGui::Begin("Basic info");
  size_t rendered = 0;
  fboMS->setActive();
	glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int cx = (int) gs->selfPosition.x >> 4;
  int cy = (int) gs->selfPosition.y >> 4;
  int cz = (int) gs->selfPosition.z;
  for (int ix = -RADIUS; ix <= RADIUS; ++ix) {
    for (int iy = -RADIUS; iy <= RADIUS; ++iy) {
      x801::map::ChunkXYZ c = { cx + ix, cy + iy, cz };
      ChunkBuffer* cmb = summon(c);
      if (cmb != nullptr) {
        cmb->render();
        ++rendered;
      }
    }
  }
  if (isChatWindowOpen) {
    ImGui::TextWrapped("%zu chunks rendered", rendered);
  }
  ImGui::End();
#ifndef NDEBUG
  glEnable(GL_DEPTH_TEST);
  axes.render();
#endif
}

static const char* VERTEX_SOURCE =
  "#version 330 core \n"
  // The position of the vertex, in 16-metre units,
  // from the centre of the northwestmost block of the chunk.
  "layout (location = 0) in vec2 position; \n"
  "layout (location = 1) in uint block; \n"
  "layout (location = 2) in uint W; \n"
  "layout (location = 3) in int dec; \n"
  "out vec2 TexCoord; \n"
  "flat out int decorator; \n"
  "flat out uint w; \n"
  // The model-view-projection matrix.
  "uniform mat4 mvp; \n"
  // The offset of this chunk, in metres.
  "uniform vec3 offset; \n"
  "void main() { \n"
  "  uvec2 localOffset = uvec2(block >> 4u, block & 15u); \n"
  "  vec3 basePosition = vec3(position + localOffset, 0) + offset; \n"
  "  gl_Position = mvp * vec4(basePosition, 1.0f); \n"
  "  if (dec != 0) gl_Position.z -= 1; \n"
  "  TexCoord = position; \n"
  "  w = W; \n"
  "  decorator = dec; \n"
  "} \n"
  ;

static const char* FRAGMENT_SOURCE =
  "#version 330 core \n"
  "in vec2 TexCoord; \n"
  "flat in uint w; \n"
  "flat in int decorator; \n"
  "out vec4 colour; \n"
  "uniform sampler2D texb; \n"
  "uniform sampler2D texd; \n"
  "#define DIVISOR 128u \n" // 4096 / 32
  "void main() { \n"
  //"  if (decorator == 0) discard; \n"
  "  vec2 uvstart = vec2(w % DIVISOR, w / DIVISOR); \n"
  // or (w / DIVISOR) % DIVISOR
  // (if we need to use multiple textures)
  "  vec2 local = mod(TexCoord, 1); \n"
  "  vec2 realtc = (local + uvstart) / DIVISOR; \n"
  // "  colour = vec4(vec3(w % 4u, (w / 4u) % 4u, (w / 16u) % 4u) / 3.0, 1); \n"
  "  colour = texture(decorator != 0 ? texd : texb, realtc); \n"
  "} \n"
  ;

void x801::game::ChunkBuffer::createMesh() {
  // Iterate over all chunk-local coordinates
  for (size_t x = 0; x < 16; ++x) {
    for (size_t y = 0; y < 16; ++y) {
      x801::map::Block block =
        chunk->getMapBlockAt(x, y);
      uint32_t baseID = block.getBaseID();
      uint32_t decorationID = block.getDecorationID();
      x801::map::BlockTextureBindings** bindings
        = tr->cw->bindings;
      if (baseID != 0 && baseID <= bindings[0]->count()) {
        size_t texID = bindings[0]->getTexID(baseID);
        MeshEntry me = {
          (uint16_t) texID,
          (uint8_t) ((x << 4) | y),
          false
        };
        mesh.push_back(me);
      }
      if (decorationID != 0 && decorationID <= bindings[1]->count()) {
        size_t texID = bindings[1]->getTexID(decorationID);
        MeshEntry me = {
          (uint16_t) texID,
          (uint8_t) ((x << 4) | y),
          true
        };
        mesh.push_back(me);
      }
    }
  }
  /*
  std::cout << "(" << chunk->getX() << ", " << chunk->getY() << ") ";
  std::cout << mesh.size() << " [";
  for (auto& m : mesh) {
    std::cout << "(" << m.w << ", " << (m.xy >> 4) << ", " << (m.xy & 15) << ", " << m.decorator << "), ";
  }
  std::cout << "]\n";
  */
}

void x801::game::ChunkBuffer::setUpRender() {
  createMesh();
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
    1, GL_UNSIGNED_BYTE,
    sizeof(MeshEntry), (void*) offsetof(MeshEntry, xy));
  glVertexAttribIPointer(
    2,
    1, GL_UNSIGNED_SHORT,
    sizeof(MeshEntry), (void*) offsetof(MeshEntry, w));
  glVertexAttribIPointer(
    3,
    1, GL_UNSIGNED_BYTE,
    sizeof(MeshEntry), (void*) offsetof(MeshEntry, decorator));
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(1, 1);
  glVertexAttribDivisor(2, 1);
  glVertexAttribDivisor(3, 1);
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
  setup = true;
#endif
}

void x801::game::ChunkBuffer::render() {
#ifndef NDEBUG
  if (!setup)
    throw "ChunkBuffer: render() called before setUpRender()";
#endif
  /*glEnable(GL_DEPTH_TEST);
  if (layer) {
    glEnable(GL_BLEND);
    glDepthMask(false);
  } else {
    glDisable(GL_BLEND);
  }*/
  tr->texb->bindTo(0);
  tr->texd->bindTo(1);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(false);
  glEnable(GL_BLEND);
  vao.setActive();
  program.use();
  glm::mat4 mvp = tr->cw->mvp;
#ifndef NDEBUG
  tr->axes.setMVP(mvp);
#endif
  SETUNSPM(program, 4fv, "mvp", glm::value_ptr(mvp));
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, mesh.size());
  glDepthMask(true);
}