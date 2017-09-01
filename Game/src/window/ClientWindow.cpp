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

#include <sstream>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include "Server.h"
#include "window/imgui_hooks.h"

extern agl::GLFWApplication* agl::currentApp;

void x801::game::ClientWindow::initialise() {
  std::cerr << "x801::game::ClientWindow::initialise();\n";
  glfwSetInputMode(underlying(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(underlying(), x801::game::customKeyCallback);
  glfwSetMouseButtonCallback(underlying(), ImGui_ImplGlfwGL3_MouseButtonCallback);
  glfwSetScrollCallback(underlying(), ImGui_ImplGlfwGL3_ScrollCallback);
  glfwSetCharCallback(underlying(), ImGui_ImplGlfwGL3_CharCallback);
  ImGui_ImplGlfwGL3_Init(underlying(), false);
  ImGuiIO& io = ImGui::GetIO();
  //io.Fonts->AddFontFromFileTTF("/home/uruwi/kiloji/kiloji_p.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  io.Fonts->AddFontFromFileTTF("intrinsic-assets/VLGothic/VL-PGothic-Regular.ttf", 18.0f, nullptr, x801::game::range);
  chat = new ChatWindow(this);
  ft = std::move(agl::makeFBOForMeMS(getWidth(), getHeight()));
  tr = new TerrainRenderer(this, ft);
  terrain = new agl::Sprite2D(&*(ft.ss.texture));
  terrain->setApp(this);
  terrain->addSprite({
    0, 0, (float) getWidth(), (float) getHeight(),
    0, 0, (float) getWidth(), (float) getHeight(),
  });
  terrain->setUp();
  fuck = new agl::Sprite2D(tr->tex);
  fuck->setApp(this);
  fuck->addSprite({
    0, 0, (float) tr->tex->getWidth(), (float) tr->tex->getHeight(),
    0, 0, (float) tr->tex->getWidth(), (float) tr->tex->getHeight(),
  });
  fuck->setUp();
  for (size_t i = 0; i < FTIMES_TO_STORE; ++i) {
    ftimes[i] = 100.0f;
  }
  std::stringstream bFile =
    c->patcher->getSStream("textures/terrain/blocks.tti");
  bindings = new x801::map::BlockTextureBindings(bFile);
}

static const int keycodes[] = {
  GLFW_KEY_UP,
  GLFW_KEY_DOWN,
  GLFW_KEY_LEFT,
  GLFW_KEY_RIGHT,
};

static const char* dirNames[] = {
  "right", "up", "left", "down"
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
  c->g.historyMutex.lock();
  RakNet::Time t = RakNet::GetTime();
  KeyInput ki = { t, inputs };
  c->g.history.pushBack(ki);
  c->g.historyMutex.unlock();
  c->g.fastForwardSelfClient(ki);
  c->sendKeyInput(ki);
  ImGui_ImplGlfwGL3_NewFrame();
  glClearColor(1.0f, 0.8f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  tr->draw();
  ft.ms.fbo->blitTo(*(ft.ss.fbo), getWidth(), getHeight());
  agl::setDefaultFBOAsActive();
  terrain->tick();
  fuck->tick();
  chat->render();
  ImGui::Begin("Basic info");
  ImGui::TextWrapped("Engine Version: %s",
    x801::base::engineVersion.toString().c_str());
  curr = (curr + 1) % FTIMES_TO_STORE;
  ftimes[curr] = 1000.0f / getFPS();
  ImGui::TextWrapped("FPS: %.2f", getRollingFPS());
  ImGui::PlotLines(
    "",
    ftimes, FTIMES_TO_STORE, curr + 1,
    "Frame times (0 to 100)", 0.0f, 100.0f, ImVec2(0, 200)
  );
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
  c->g.locationMutex.lock_shared();
  for (const auto& pair : c->g.playersByID) {
    uint32_t id = pair.first;
    const Location& loc =
      (id == c->g.myID) ? pair.second.getLocation() : c->g.selfPosition;
    ImGui::TextWrapped(
      "%s (#%d) @ world-%d area-%d (%f, %f, %d) < %s",
      c->getUsername(id).c_str(), id,
      loc.areaID.worldID, loc.areaID.areaID,
      loc.x, loc.y, loc.z,
      dirNames[loc.rot]
    );
  }
  c->g.locationMutex.unlock_shared();
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
  delete tr;
  delete terrain;
  delete fuck;
  delete bindings;
}