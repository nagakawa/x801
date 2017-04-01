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

using namespace x801::game;

x801::game::TerrainRenderer::TerrainRenderer(ClientWindow* cw) {
  this->cw = cw;
  c = cw->c;
  p = c->patcher;
  tv = c->textureView;
  gs = &(c->g);
  ModelView* mv = c->modelView;
  mai = mv->getMAI();
  mfi = mv->getMFI();
  assert(cw != nullptr && c != nullptr && p != nullptr && tv != nullptr && gs != nullptr);
}

x801::map::Chunk* x801::game::TerrainRenderer::getChunk(const x801::map::ChunkXYZ& pos) {
  x801::map::Area* a = gs->currentArea.getArea();
  assert(a != nullptr);
  x801::map::TileSec& ts = a->getTileSec();
  auto it = ts.getChunks().find(pos);
  if (it == ts.getChunks().end()) return nullptr;
  return &(it->second);
}

x801::game::ChunkMeshBuffer::ChunkMeshBuffer(const x801::map::ChunkXYZ& xyz, TerrainRenderer* tr) {
  this->xyz = xyz;
  this->tr = tr;
  chunk = tr->getChunk(xyz);
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
      vertices.push_back(triangle[j]);
    }
  }
}