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

using namespace x801::game;

x801::game::TerrainRenderer::TerrainRenderer(ClientWindow* cw, agl::FBOTexMS& ft) {
  this->cw = cw;
  c = cw->c;
  p = c->patcher;
  tv = c->textureView;
  gs = &(c->g);
  tex = tv->getTexture("textures/terrain/blocks.0.png");
  ModelView* mv = c->modelView;
  mai = mv->getMAI();
  mfi = mv->getMFI();
  assert(
    cw != nullptr && c != nullptr &&
    p != nullptr && tv != nullptr &&
    gs != nullptr && tex != nullptr);
  fboMS = ft.ms.fbo;
  bindings = cw->bindings;
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
  res->setUpRender(false);
  res->setUpRender(true);
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
        cmb->render(false);
        //cmb->render(true);
        ++rendered;
        if (isChatWindowOpen) {
          ImGui::TextWrapped("%d, %d rendered", c.x, c.y);
        }
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

float squareCoords[6][2] = {
  {0.0f, 0.0f},
  {0.0f, 1.0f},
  {1.0f, 0.0f},
  {0.0f, 1.0f},
  {1.0f, 0.0f},
  {1.0f, 1.0f},
};

static const char* VERTEX_SOURCE =
  "#version 330 core \n"
  // The position of the vertex, in 16-metre units,
  // from the centre of the northwestmost block of the chunk.
  "layout (location = 0) in vec2 position; \n"
  "out vec2 TexCoord; \n"
  // The model-view-projection matrix.
  "uniform mat4 mvp; \n"
  // The offset of this chunk, in metres.
  "uniform vec3 offset; \n"
  "void main() { \n"
  "  gl_Position = mvp * vec4(vec3(16 * position, 0) + offset, 1.0f); \n"
  "  TexCoord = position; \n"
  "} \n"
  ;

static const char* FRAGMENT_SOURCE =
  "#version 330 core \n"
  "in vec2 TexCoord; \n"
  "out vec4 colour; \n"
  "uniform sampler2D tex; \n"
  "uniform uint tiles[256]; \n"
  "uniform uint mappings[]; \n"
  "#define DIVISOR 128u \n" // 4096 / 32
  "void main() { \n"
  "  ivec2 indices = ivec2(TexCoord * 16); \n"
  "  uint w = tiles[indices.y * 16 + indices.x]; \n"
  "  uint base = w & 0xFFFFu/*ck*/; \n"
  "  if (base == 0u) discard; \n"
  "  uint baseTexID = mappings[base - 1u]; \n"
  "  vec2 uvstart = vec2(base % DIVISOR, base / DIVISOR); \n"
  // or (W / DIVISOR) % DIVISOR
  // (if we need to use multiple textures)
  "  vec2 local = mod(TexCoord * 16, 1); \n"
  "  vec2 realtc = (local + uvstart) / DIVISOR; \n"
  "  colour = texture(tex, realtc); \n"
  "} \n"
  ;

void x801::game::ChunkBuffer::setUpRender(bool layer) {
  agl::Shader* vertexShader = new agl::Shader(VERTEX_SOURCE, GL_VERTEX_SHADER);
  agl::Shader* fragmentShader = new agl::Shader(FRAGMENT_SOURCE, GL_FRAGMENT_SHADER);
  program[layer].attach(*vertexShader);
  program[layer].attach(*fragmentShader);
  program[layer].link();
  program[layer].use();
  delete vertexShader;
  delete fragmentShader;
  vao[layer].setActive();
  vbo[layer].feedData(sizeof(squareCoords), squareCoords, GL_STATIC_DRAW);
  // Set vertex coordinates of rect
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  program[layer].use();
  tr->tex->bindTo(0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  SETUNSP(program[layer], 1i, "tex", 0);
  glm::vec3 offset(
    xyz.x * 16.0f,
    xyz.y * 16.0f,
    xyz.z
  );
  SETUNSPV(program[layer], 3fv, "offset", glm::value_ptr(offset));
  glUniform1uiv(
    program[layer].getUniformLocation("tiles"),
    16 * 16,
    (GLuint*) chunk->getMapBlocks()
  );
  glUniform1uiv(
    program[layer].getUniformLocation("mappings"),
    tr->cw->bindings->count(),
    (GLuint*) tr->cw->bindings->data()
  );
  std::cout << "\n";
#ifndef NDEBUG
  setup[layer] = true;
#endif
}

void x801::game::ChunkBuffer::render(bool layer) {
#ifndef NDEBUG
  if (!setup[layer])
    throw "ChunkBuffer: render() called before setUpRender()";
#endif
  /*glEnable(GL_DEPTH_TEST);
  if (layer) {
    glEnable(GL_BLEND);
    glDepthMask(false);
  } else {
    glDisable(GL_BLEND);
  }*/
  glEnable(GL_DEPTH_TEST);
  glDepthMask(false);
  glEnable(GL_BLEND);
  vao[layer].setActive();
  program[layer].use();
  glm::mat4 mvp;
  tr->gs->selfPositionMutex.lock();
  const auto selfPos = tr->gs->selfPosition;
  tr->gs->selfPositionMutex.unlock();
  float theta = selfPos.rot;
  float aspectRatio = ((float) tr->cw->getWidth()) / tr->cw->getHeight();
#if 0 // top-downish perspective code
  // I don't know why I have to multiply the y by -1, but it works.
  mvp = glm::scale(mvp, glm::vec3(1.0f / aspectRatio, -1.0f, -1.0f) / 8.0f);
  // Rotate 30 degrees backwards
  mvp = glm::rotate(mvp, glm::radians(30.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
  mvp = glm::rotate(mvp, glm::radians(90.0f) - theta, glm::vec3(0.0f, 0.0f, 1.0f));
  // Centre on player
  mvp = glm::translate(
    mvp,
    glm::vec3(
      -selfPos.x,
      -selfPos.y,
      -selfPos.z
    )
  );
#endif
  mvp = glm::scale(mvp, glm::vec3(1.0f, -1.0f, 1.0f));
  mvp *= glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
  mvp *= glm::lookAt(
    glm::vec3(selfPos.x, selfPos.y, selfPos.z + 1.6f),
    glm::vec3(selfPos.x + cosf(theta), selfPos.y + sinf(theta), selfPos.z + 1.6f),
    glm::vec3(0.0f, 0.0f, 1.0f)
  );
#ifndef NDEBUG
  tr->axes.setMVP(mvp);
#endif
  SETUNSPM(program[layer], 4fv, "mvp", glm::value_ptr(mvp));
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDepthMask(true);
}