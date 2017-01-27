#include "ClientWindow.h"

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

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

extern agl::GLFWApplication* agl::currentApp;

static void customKeyCallback(
    GLFWwindow* window, int key, int scancode, int action, int mode) {
  ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mode);
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) return;
  if (action == GLFW_PRESS) agl::currentApp->setKey(key);
	else if (action == GLFW_RELEASE) agl::currentApp->resetKey(key);
}

void x801::game::ClientWindow::initialise() {
  std::cerr << "x801::game::ClientWindow::initialise();\n";
  glfwSetInputMode(underlying(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(underlying(), customKeyCallback);
  glfwSetMouseButtonCallback(underlying(), ImGui_ImplGlfwGL3_MouseButtonCallback);
  glfwSetScrollCallback(underlying(), ImGui_ImplGlfwGL3_ScrollCallback);
  glfwSetCharCallback(underlying(), ImGui_ImplGlfwGL3_CharCallback);
  ImGui_ImplGlfwGL3_Init(underlying(), false);
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/vlgothic/VL-PGothic-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
}

void x801::game::ClientWindow::tick() {
  if (c->isDone() || glfwWindowShouldClose(underlying())) {
    glfwSetWindowShouldClose(underlying(), true);
  }
  ImGui_ImplGlfwGL3_NewFrame();
  glClearColor(1.0f, 0.8f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui::Render();
}

void x801::game::ClientWindow::readKeys() {

}

void x801::game::ClientWindow::onMouse(double xpos, double ypos) {
  (void) xpos; (void) ypos;
}

x801::game::ClientWindow::~ClientWindow() {
  ImGui_ImplGlfwGL3_Shutdown();
}