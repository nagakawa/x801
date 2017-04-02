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
#include <utility>

#include <imgui.h>

using namespace x801::game;

x801::game::TerrainRenderer::TerrainRenderer(ClientWindow* cw) {
  this->cw = cw;
  c = cw->c;
  p = c->patcher;
  tv = c->textureView;
  gs = &(c->g);
  tex = std::move(tv->getTexture("textures/terrain/blocks.png"));
  ModelView* mv = c->modelView;
  mai = mv->getMAI();
  mfi = mv->getMFI();
  assert(cw != nullptr && c != nullptr && p != nullptr && tv != nullptr && gs != nullptr && tex != nullptr);
  agl::FBOTexMS ft = agl::makeFBOForMeMS(1280, 960);
  fboTex = ft.ss.texture;
  fboTexMS = ft.ms.texture;
  fboSS = ft.ss.fbo;
  fboMS = ft.ms.fbo;
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

ChunkMeshBuffer* x801::game::TerrainRenderer::summon(const x801::map::ChunkXYZ& pos) {
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
  res->createMesh();
  res->setUpRender(false);
  res->setUpRender(true);
  return res;
}

static const int RADIUS = 5;

void x801::game::TerrainRenderer::draw() {
  size_t rendered = 0;
  fboMS->setActive();
	glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int cx = (int) gs->selfPosition.x >> 4;
  int cy = (int) gs->selfPosition.y >> 4;
  int cz = (int) gs->selfPosition.z >> 4;
  for (int ix = -RADIUS; ix < RADIUS; ++ix) {
    for (int iy = -RADIUS; iy < RADIUS; ++iy) {
      for (int iz = -RADIUS; iz < RADIUS; ++iz) {
        x801::map::ChunkXYZ c = { cx + ix, cy + iy, cz + iz };
        ChunkMeshBuffer* cmb = summon(c);
        if (cmb != nullptr) {
          cmb->render(false);
          cmb->render(true);
          ++rendered;
        }
      }
    }
  }
  (void) rendered;
#ifndef NDEBUG
  std::cerr << rendered << " chunks rendered\n";
#endif
}

void x801::game::ChunkMeshBuffer::createMesh() {
  opaqueVertices.clear();
  transparentVertices.clear();
  for (size_t lx = 0; lx < 16; ++lx) {
    for (size_t ly = 0; ly < 16; ++ly) {
      for (size_t lz = 0; lz < 16; ++lz)
        addBlock(lx, ly, lz);
    }
  }
}

void x801::game::ChunkMeshBuffer::addBlock(size_t lx, size_t ly, size_t lz) {
  using namespace x801::map;
  Block b = chunk->getMapBlockAt(lx, ly, lz);
  if (b.label == 0) return; // it is air
  ModelApplication& ma = tr->mai->applications[b.label - 1];
  ModelFunction& mf = tr->mfi->models[ma.modfnum];
  CMVertex triangle[3];
  size_t disturbed[18] = {
    lx, ly, lz + 1, lx, ly, lz - 1,
    lx, ly + 1, lz, lx, ly - 1, lz,
    lx + 1, ly, lz, lx - 1, ly, lz
  };
  for (const Face& face : mf.faces) {
    uint8_t flags = face.occlusionFlags;
    bool occluded = false;
    for (size_t i = 0; i < 6; ++i) {
      if ((flags & (1 << i)) == 0) continue;
      // If any of them are out of range, they will either be 16 or the max value of size_t.
      if (disturbed[3 * i] >= 16 || disturbed[3 * i + 1] >= 16 || disturbed[3 * i + 2] >= 16)
        continue;
      Block db = chunk->getMapBlockAt(
        (size_t) disturbed[3 * i],
        (size_t) disturbed[3 * i + 1],
        (size_t) disturbed[3 * i + 2]
      );
      if (db.label == 0) continue;
      // TODO maybe cache the opacity flags?
      ModelApplication& dma = tr->mai->applications[db.label - 1];
      ModelFunction dmf = tr->mfi->models[dma.modfnum];
      if ((dmf.opacityFlags & (1 << (i ^ 1))) == 0) {
        occluded = true;
        break;
      }
    }
    if (occluded) continue;
    size_t textureVOffset = ma.textures[face.texture] << 7;
    for (size_t j = 0; j < 3; ++j) {
      uint16_t index = face.vertices[j].index;
      VertexXYZ& blvertex = mf.vertices[index];
      triangle[j].x = (lx << 7) + blvertex.x;
      triangle[j].y = (ly << 7) + blvertex.y;
      triangle[j].z = (lz << 7) + blvertex.z;
      triangle[j].u = face.vertices[j].u;
      triangle[j].v = face.vertices[j].v + textureVOffset;
      if (flags & 64)
        transparentVertices.push_back(triangle[j]);
      else
        opaqueVertices.push_back(triangle[j]);
    }
  }
}

static const char* VERTEX_SOURCE =
  "#version 330 core \n"
  "layout (location = 0) in ivec3 position; \n"
  "layout (location = 1) in int u; \n"
  "layout (location = 2) in uint v; \n"
  "out vec2 TexCoord; \n"
  "uniform mat4 mvp; \n"
  // How many times taller the texture is than wide.
  "uniform float dim; \n"
  "void main() { \n"
  "  gl_Position = mvp * vec4(position / 128.0, 1); \n"
  "  TexCoord = vec2(u / 128.0, v / (dim * 128.0)); \n"
  "} \n"
  ;

static const char* FRAGMENT_SOURCE =
  "#version 330 core \n"
  "in vec2 TexCoord; \n"
  "out vec4 colour; \n"
  "uniform sampler2D tex; \n"
  "void main() { \n"
  "  colour = texture(tex, TexCoord); \n"
  //"  colour = vec4(TexCoord.x * 0.0f, TexCoord.y * 8.0f, 0.0f, 1.0f); \n"
  "} \n"
  ;

void x801::game::ChunkMeshBuffer::setUpRender(bool layer) {
  agl::Shader* vertexShader = new agl::Shader(VERTEX_SOURCE, GL_VERTEX_SHADER);
  agl::Shader* fragmentShader = new agl::Shader(FRAGMENT_SOURCE, GL_FRAGMENT_SHADER);
  program[layer].attach(*vertexShader);
  program[layer].attach(*fragmentShader);
  program[layer].link();
  program[layer].use();
  delete vertexShader;
  delete fragmentShader;
  vao[layer].setActive();
  if (layer)
    vbo[layer].feedData(transparentVertices.size() * sizeof(CMVertex), transparentVertices.data(), GL_STATIC_DRAW);
  else
    vbo[layer].feedData(opaqueVertices.size() * sizeof(CMVertex), opaqueVertices.data(), GL_STATIC_DRAW);
  // Set xyz
  glVertexAttribIPointer(0, 3, GL_SHORT, sizeof(CMVertex), (void*) offsetof(CMVertex, x));
  // Set u
  glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(CMVertex), (void*) offsetof(CMVertex, u));
  // Set v
  glVertexAttribIPointer(2, 1, GL_INT, sizeof(CMVertex), (void*) offsetof(CMVertex, v));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  program->use();
  tr->tex->bindTo(0);
  SETUNSP(program[layer], 1i, "tex", 0);
  SETUNSP(program[layer], 1f, "dim", ((float) tr->tex->getHeight()) / tr->tex->getWidth());
#ifndef NDEBUG
  setup[layer] = true;
#endif
}

void x801::game::ChunkMeshBuffer::render(bool layer) {
#ifndef NDEBUG
  if (!setup[layer])
    throw "ChunkMeshBuffer: render() called before setUpRender()";
#endif
  if (layer) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
  vao[layer].setActive();
  program[layer].use();
  glm::mat4 mvp;
  mvp = glm::translate(
    mvp,
    glm::vec3(
      xyz.x * -16.0f,
      xyz.y * -16.0f,
      xyz.z * -16.0f
    )
  );
  const auto selfPos = tr->gs->selfPosition;
  float theta = selfPos.rot;
  // Centre on player
  mvp = glm::translate(
    mvp,
    glm::vec3(
      -selfPos.x,
      -selfPos.y,
      -selfPos.z
    )
  );
  bool isChatWindowOpen = ImGui::Begin("Basic info");
  if (isChatWindowOpen) {
    ImGui::TextWrapped("Self theta: %f", (double) theta);
  }
  ImGui::End();
  // Rotate by theta clockwise
  mvp = glm::rotate(mvp, -theta, glm::vec3(0.0f, 0.0f, 1.0f));
  // mvp = glm::rotate(mvp, (float) (-0.8 * glfwGetTime()), glm::vec3(0.0f, 0.0f, 1.0f));
  // (void) theta;
  // Rotate 30 degrees backward
  //mvp = glm::rotate(mvp, glm::radians(30.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
  float aspectRatio = ((float) tr->cw->getWidth()) / tr->cw->getHeight();
  mvp = glm::scale(mvp, glm::vec3(1.0f / aspectRatio, 1.0f, 1.0f) / 8.0f);
  SETUNSPM(program[layer], 4fv, "mvp", glm::value_ptr(mvp));
  size_t count = layer ? transparentVertices.size() : opaqueVertices.size();
  glDrawArrays(GL_TRIANGLES, 0, count);
}