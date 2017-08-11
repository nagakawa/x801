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

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <FBO.h>
#include <VAO.h>
#include <VBO.h>

#include "sizelimit.h"

namespace x801 {
  namespace game {
    class Atlas {
    public:
      Atlas();
      struct Location {
        uint32_t page;
        uint32_t left, top, right, bottom;
      };
      class Page {
      public:
        class Node;
        class Node {
        public:
          Node(size_t left, size_t top, size_t right, size_t bottom) :
            left(left), top(top), right(right), bottom(bottom) {}
          Node(const Node& other) = delete;
          Node(Node&& other) :
              child0(other.child0), child1(other.child1),
              left(other.left), top(other.top),
              right(other.right), bottom(other.bottom),
              occupied(other.occupied) {
            other.child0 = nullptr;
            other.child1 = nullptr;
          }
          Node& operator=(const Node& other) = delete;
          ~Node();
          Node* child0 = nullptr;
          Node* child1 = nullptr;
          uint32_t left, top, right, bottom;
          bool occupied = false;
          Node* insert(size_t w, size_t h);
        };
        Page() :
          fbo(agl::makeFBOForMe(texsize, texsize)),
          node(0, 0, texsize, texsize) {}
        Page(const Page& other) = delete;
        Page(Page&& other) :
          fbo(std::move(other.fbo)), node(std::move(other.node)) {}
        agl::FBOTex fbo;
        Node node;
        void burn(Node& node, const agl::Texture& tex);
      };
      std::vector<Page> pages;
      std::unordered_map<std::string, Location> locations;
      agl::VAO vao;
      agl::VBO vbo;
      void insert(const std::string& name, const agl::Texture& tex);
      void setUpRender();
    };
  }
}