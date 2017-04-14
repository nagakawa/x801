#include "window/Axes.h"

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

using namespace x801::game;

const float F_INF = 1.0f;
const float F_NINF = -1.0f;

static float AXES_DEF[][2][6] = {
  { // X+
    {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
    {F_INF, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f}
  },
  { // X-
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    {F_NINF, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f}
  },
  { // Y+
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, F_INF, 0.0f, 0.5f, 1.0f, 0.5f}
  },
  { // Y-
    {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, F_NINF, 0.0f, 1.0f, 0.5f, 1.0f}
  },
  { // Z+
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, F_INF, 0.5f, 0.5f, 1.0f}
  },
  { // Z-
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, F_NINF, 1.0f, 1.0f, 0.5f}
  },
};

static const char* VERTEX_SOURCE =
  "#version 330 core \n"
  "layout (location = 0) in ivec3 position; \n"
  "layout (location = 1) in ivec3 inColour; \n"
  "out vec3 outColour; \n"
  "uniform mat4 mvp; \n"
  "uniform vec3 myPos; \n"
  "uniform float scale; \n"
  "void main() { \n"
  "  gl_Position = mvp * vec4(position + myPos, 1.0f); \n"
  "  outColour = inColour; \n"
  "} \n"
  ;

static const char* FRAGMENT_SOURCE =
  "#version 330 core \n"
  "in vec3 outColour; \n"
  "out vec4 colour; \n"
  "uniform sampler2D tex; \n"
  "void main() { \n"
  "  colour = vec4(outColour, 1.0f); \n"
  "} \n"
  ;

x801::game::Axes::Axes() {
}

void x801::game::Axes::setUpRender() {
  agl::Shader* vertexShader = new agl::Shader(VERTEX_SOURCE, GL_VERTEX_SHADER);
  agl::Shader* fragmentShader = new agl::Shader(FRAGMENT_SOURCE, GL_FRAGMENT_SHADER);
  program.attach(*vertexShader);
  program.attach(*fragmentShader);
  program.link();
  delete vertexShader;
  delete fragmentShader;
  vao.setActive();
  vbo.feedData(sizeof(AXES_DEF), (void*) AXES_DEF, GL_STATIC_DRAW);
  // XYZ
  glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(AXES_DEF[0][0]), (void*) 0);
  // RGB
  glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(AXES_DEF[0][0]), (void*) (3 * sizeof(AXES_DEF[0][0][0])));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  program.use();
}

void x801::game::Axes::render() {
  glEnable(GL_DEPTH_TEST);
  vao.setActive();
  program.use();
  SETUNSPM(program, 4fv, "mvp", glm::value_ptr(mvp));
  SETUNSPV(program, 3fv, "myPos", glm::value_ptr(pos));
  SETUNSP(program, 1f, "scale", scale);
  glDrawArrays(GL_LINES, 0, sizeof(AXES_DEF) / sizeof(AXES_DEF[0][0]));
}
