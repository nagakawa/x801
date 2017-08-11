#include "window/entity_rendering/Atlas.h"

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

#include <Shader.h>
#include <ShaderProgram.h>

namespace x801 {
  namespace game {
    static const char* ATLAS_VERTEX =
      "#version 330 core \n"
      "layout (location = 0) in vec2 vertex; \n"
      "uniform vec4 bounds;"
      "out vec2 uv; \n"
      "void main() { \n"
      "  vec2 lp = vec2(vertex.x, 1 - vertex.y); \n"
      "  vec2 tp = mix(bounds.xy, bounds.zw, lp); \n"
      "  gl_Position = vec2(-1, 1) + vec2(2, -2) * tp; \n"
      "  uv = lp;"
      "}"
      "";
    static const char* ATLAS_FRAGMENT =
      "#version 330 core \n"
      "in vec2 uv; \n"
      "out vec4 colour; \n"
      "uniform sampler2D tex; \n"
      "void main { \n"
      "  colour = texture(tex, uvout);"
      "}"
      "";
    static const float ATLAS_VERTEX_DATA[] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
    };
    static agl::ShaderProgram* atlasShader = nullptr;
    static void initialiseAtlasShader() {
      agl::Shader vert(ATLAS_VERTEX, GL_VERTEX_SHADER);
      agl::Shader frag(ATLAS_FRAGMENT, GL_FRAGMENT_SHADER);
      atlasShader = new agl::ShaderProgram();
      atlasShader->attach(vert);
      atlasShader->attach(frag);
      atlasShader->link();
    }

    inline size_t fits(
        size_t rw, size_t rh,
        size_t w, size_t h) {
      if (w > rw || h > rh) return 0;
      if (w == rw && h == rh) return 1;
      return 2;
    }
    Atlas::Atlas() {
      getTexsize();
    }
    Atlas::Page::Node::~Node() {
      delete child0;
      delete child1;
    }
    // Thanks http://blackpawn.com/texts/lightmaps/default.html for the algo
    Atlas::Page::Node* Atlas::Page::Node::insert(size_t w, size_t h) {
      if (child0 != nullptr || child1 != nullptr) {
        Node* newNode = child0->insert(w, h);
        if (newNode != nullptr) return newNode;
        return child1->insert(w, h);
      }
      if (occupied) return nullptr;
      size_t rw = right - left;
      size_t rh = bottom - top;
      size_t fitScore = fits(rw, rh, w, h);
      if (fitScore == 0) return nullptr;
      if (fitScore == 1) {
        occupied = true;
        return this;
      }
      // Split into 2 nodes
      size_t dw = rw - w;
      size_t dh = rh - h;
      if (dw > dh) {
        child0 = new Node(left, top, left + w, top);
        child1 = new Node(left + w, top, right, top);
      } else {
        child0 = new Node(left, top, left, top + h);
        child1 = new Node(left, top + h, right, bottom);
      }
      return child0->insert(w, h);
    }
    void Atlas::Page::burn(Node& node, const agl::Texture& tex) {
      atlasShader->use();
      tex.bindTo(0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      SETUNSP(*atlasShader, 1i, "tex", 0);
      float bounds[4] = {
        (float) node.left / texsize, (float) node.top / texsize,
        (float) node.right / texsize, (float) node.bottom / texsize
      };
      SETUNSPV(*atlasShader, 4fv, "bounds", bounds);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void Atlas::insert(const std::string& name, const agl::Texture& tex) {
      Page::Node* node;
      uint32_t i;
      for (i = 0; i < pages.size(); ++i) {
        node = pages[i].node.insert(tex.getWidth(), tex.getHeight());
        if (node != nullptr) break;
      }
      pages[i].burn(*node, tex);
      locations[name] =
        { i, node->left, node->top, node->right, node->bottom };
    }
    void Atlas::setUpRender() {
      if (atlasShader == nullptr)
        initialiseAtlasShader();
      vao.setActive();
      vbo.feedData(sizeof(ATLAS_VERTEX_DATA), ATLAS_VERTEX_DATA, GL_STATIC_DRAW);
      // Set xyz
      glVertexAttribIPointer(0, 2, GL_FLOAT, 2 * sizeof(float), (void*) 0);
      glEnableVertexAttribArray(0);
    }
  }
}