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

#include <sstream>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <XDatSec.h>
#include "Server.h"
#include "window/imgui_hooks.h"

extern agl::GLFWApplication* agl::currentApp;

namespace x801 {
  namespace game {
    void ClientWindow::initialise() {
      std::cerr << "x801::game::ClientWindow::initialise();\n";
      glfwSetInputMode(underlying(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      glfwSetKeyCallback(underlying(), customKeyCallback);
      glfwSetMouseButtonCallback(underlying(), ImGui_ImplGlfwGL3_MouseButtonCallback);
      glfwSetScrollCallback(underlying(), ImGui_ImplGlfwGL3_ScrollCallback);
      glfwSetCharCallback(underlying(), ImGui_ImplGlfwGL3_CharCallback);
      ImGui_ImplGlfwGL3_Init(underlying(), false);
      ImGuiIO& io = ImGui::GetIO();
      //io.Fonts->AddFontFromFileTTF("/home/uruwi/kiloji/kiloji_p.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
      io.Fonts->AddFontFromFileTTF(
        "intrinsic-assets/VLGothic/VL-PGothic-Regular.ttf",
        18.0f, nullptr, range);
      chat = new ChatWindow(this);
      ft = std::move(agl::makeFBOForMeMS(getWidth(), getHeight()));
      setPixelScale();
      std::stringstream bFile =
        c->patcher->getSStream("textures/terrain/blocks.tti");
      bindings[0] = new x801::map::BlockTextureBindings(bFile);
      std::stringstream bFile2 =
        c->patcher->getSStream("textures/decorations/blocks.tti");
      bindings[1] = new x801::map::BlockTextureBindings(bFile2);
      tr = new TerrainRenderer(this, ft);
      em = new EntityManager();
      er = new EntityRenderer(this, ft, em);
      er->setUpRender();
      terrain = new agl::Sprite2D(&*(ft.ss.texture));
      terrain->setApp(this);
      terrain->addSprite({
        0, 0, (float) getWidth(), (float) getHeight(),
        0, 0, (float) getWidth(), (float) getHeight(),
      });
      terrain->setUp();
      fuck = new agl::Sprite2D(tr->texd);
      fuck->setApp(this);
      fuck->addSprite({
        0, 0, (float) tr->texd->getWidth(), (float) tr->texd->getHeight(),
        0, 0, (float) tr->texd->getWidth(), (float) tr->texd->getHeight(),
      });
      fuck->setUp();
      for (size_t i = 0; i < FTIMES_TO_STORE; ++i) {
        ftimes[i] = 100.0f;
      }
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

    void ClientWindow::tick() {
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
      setMVP();
      glClearColor(1.0f, 0.8f, 0.8f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      tr->draw();
      readPlayersFromGS();
      er->feed();
      er->render();
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
      x801::map::XDatSec& xs =
        c->g.getCurrentArea().getArea()->getXDatSec();
      ImGui::TextWrapped(
        "Current area: %s, %s",
        xs.worldName.c_str(), xs.areaName.c_str()
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
      em->advanceFrame();
    }

    void ClientWindow::readKeys() {

    }

    void ClientWindow::onMouse(double xpos, double ypos) {
      (void) xpos; (void) ypos;
    }

    // We want this many tiles in the window.
    static constexpr size_t WANTED_AREA = 200;

    void ClientWindow::setPixelScale() {
      pixelScale =
        x801::base::calculatePixelScale(
          TILE_SIZE,
          (size_t) getWidth(), (size_t) getHeight(),
          WANTED_AREA);
      std::cout << "Pixel scale: " << pixelScale << "\n";
    }
    
    void x801::game::ClientWindow::setMVP() {
      c->g.selfPositionMutex.lock();
      const auto selfPos = c->g.selfPosition;
      c->g.selfPositionMutex.unlock();
      // -----------------------------------------
      float aspectRatio = ((float) getWidth()) / getHeight();
      size_t pHeight = getHeight() / (pixelScale * TILE_SIZE);
      // Top edge to bottom edge: NDCs differ by 2.0f.
      float heightScale = 2.0f / pHeight;
      // -----------------------------------------
      // Read these transformations from bottom to top.
      mvp = glm::mat4();
      mvp = glm::scale(mvp, glm::vec3(1.0f / aspectRatio, 1.0f, -1.0f) * heightScale);
      // Round player coordinates to the nearest 1/16th of a block
      float spx = roundf(selfPos.x * 16) / 16;
      float spy = roundf(selfPos.y * 16) / 16;
      float spz = roundf(selfPos.z * 16) / 16;
      // Centre on player
      mvp = glm::translate(
        mvp,
        glm::vec3(-spx, -spy, -spz)
      );
    }

    void ClientWindow::readPlayersFromGS() {
      // First, go over the list of users in the entityIDsBYPlayer map
      // and remove any that don't exist in the CGS.
      std::vector<uint32_t> removeList;
      for (auto& p : entityIDsByPlayer) {
        if (c->g.playersByID.count(p.first) == 0) {
          removeList.push_back(p.first);
        }
      }
      for (uint32_t player : removeList) {
        size_t id = entityIDsByPlayer[player];
        em->deleteEntity(id);
        entityIDsByPlayer.erase(player);
      }
      // Second, go over the list of users in the ClientGameState
      // and add any new players, as well as updating
      // existing players.
      for (auto& p : c->g.playersByID) {
        
        auto it = entityIDsByPlayer.find(p.first);
        if (it == entityIDsByPlayer.end()) {
          // Add the player
          size_t id =
            em->addEntity<PlayerEntity>(p.first, p.second.getLocation());
          entityIDsByPlayer[p.first] = id;
        } else {
          size_t id = it->second;
          PlayerEntity* e = (PlayerEntity*) em->getEntity(id);
          if (p.first == c->g.myID) {
            // Is this thee, experimenter?
            e->setLocation(c->g.selfPosition);
          } else {
            e->setLocation(p.second.getLocation());
          }
        }
      }
    }

    ClientWindow::~ClientWindow() {
      delete chat;
      delete tr;
      delete er;
      delete em;
      delete terrain;
      delete fuck;
      delete bindings[0];
      delete bindings[1];
      ImGui_ImplGlfwGL3_Shutdown();
    }
  }
}
