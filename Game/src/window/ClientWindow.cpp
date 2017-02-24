#include "window/ClientWindow.h"

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
#include "Server.h"

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
  //io.Fonts->AddFontFromFileTTF("/home/uruwi/kiloji/kiloji_p.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  io.Fonts->AddFontFromFileTTF("intrinsic-assets/VLGothic/VL-PGothic-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  chat = new ChatWindow(this);
}

static const int keycodes[] = {
  GLFW_KEY_UP,
  GLFW_KEY_DOWN,
  GLFW_KEY_LEFT,
  GLFW_KEY_RIGHT,
};

static const int keycodeCount = sizeof(keycodes) / sizeof(keycodes[0]);

void x801::game::ClientWindow::tick() {
  //RakNet::TimeUS t1 = RakNet::GetTimeUS();
  if (c->isDone() || glfwWindowShouldClose(underlying())) {
    glfwSetWindowShouldClose(underlying(), true);
  }
  // Send key messages
  uint32_t inputs = 0;
  for (int i = 0; i < keycodeCount; ++i) {
    if (testKey(keycodes[i])) inputs |= (1 << i);
  }
  RakNet::Time t = RakNet::GetTime();
  KeyInput ki = { t, inputs };
  c->sendKeyInput(ki);
  c->g.history.pushBack(ki);
  ImGui_ImplGlfwGL3_NewFrame();
  glClearColor(1.0f, 0.8f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  chat->render();
  ImGui::Begin("Basic info");
  ImGui::TextWrapped("FPS: %.2f", getFPS());
  std::stringstream s;
  s << "User ID: ";
  s << c->g.getID();
  s << "\nCookie: ";
  for (int i = 0; i < COOKIE_LEN; ++i) {
    uint8_t byte = c->cookie[i];
    s << "0123456789abcdef"[byte >> 4];
    s << "0123456789abcdef"[byte & 15];
    s << ' ';
  }
  std::string str(s.str());
  ImGui::TextWrapped("%s", str.c_str());
  for (const auto& pair : c->g.playersByID) {
    uint32_t id = pair.first;
    const Location& loc =
      (id == c->g.myID) ? pair.second.getLocation() : c->g.selfPosition;
    ImGui::TextWrapped(
      "%s (#%d) @ %d-%d-%d (%f, %f) < %f radians",
      c->getUsername(id).c_str(), id,
      loc.areaID.worldID, loc.areaID.areaID,
      loc.layer,
      loc.x, loc.y, loc.rot
    );
  }
  ImGui::TextWrapped("inputs = 0x%x", inputs);
  ImGui::TextWrapped("Size of history is %zu", c->g.history.size());
  ImGui::End();
  ImGui::Render();
  //RakNet::TimeUS t2 = RakNet::GetTimeUS();
  // std::cout << "Time: " << (t2 - t1) << " FPS: " << getFPS() << " or " << ImGui::GetIO().Framerate << '\n';
}

void x801::game::ClientWindow::readKeys() {

}

void x801::game::ClientWindow::onMouse(double xpos, double ypos) {
  (void) xpos; (void) ypos;
}

x801::game::ClientWindow::~ClientWindow() {
  ImGui_ImplGlfwGL3_Shutdown();
  delete chat;
}