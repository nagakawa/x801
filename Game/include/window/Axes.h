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

namespace x801 {
  namespace game {
    class Axes {
    public:
      Axes();
      void setUpRender();
      void render();
      void setMVP(glm::mat4 mvp) { this->mvp = mvp; }
      void setPos(glm::vec3 pos) { this->pos = pos; }
      void setScale(float scale) { this->scale = scale; }
    private:
      std::shared_ptr<agl::FBO> fbo;
      agl::VBO vbo;
      agl::VAO vao;
      agl::ShaderProgram program;
      glm::mat4 mvp;
      glm::vec3 pos;
      float scale = 1.0f;
    };
  }
}
