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

#include <iostream>

#include <FBO.h>
#include <GLFWApplication.h>
#include <Sprite2D.h>
#include <pixelratio.h>
namespace x801 {
  namespace game {
    class ClientWindow;
  }
}
#include <Chunk.h>
#include "Client.h"
#include "window/ChatWindow.h"
#include "window/TerrainRenderer.h"
#include "window/entity_rendering/EntityManager.h"
#include "window/entity_rendering/EntityRenderer.h"

namespace x801 {
  namespace game {
    class ClientWindow : public agl::GLFWApplication {
    public:
      using agl::GLFWApplication::GLFWApplication;
      void initialise() override;
      void tick() override;
      void readKeys() override;
      void onMouse(double xpos, double ypos) override;
      void start() {
        GLFWApplication::start();
      }
      ChatWindow* getChatWindow() { return chat; }
      Client* getParentClient() { return c; }
      void loadEntities();
      virtual ~ClientWindow() override;
      Client* c;
      x801::map::BlockTextureBindings* bindings[2]
        = {nullptr, nullptr};
      void setPixelScale();
      size_t pixelScale;
      glm::mat4 mvp;
    private:
      ChatWindow* chat = nullptr;
      TerrainRenderer* tr = nullptr;
      EntityManager* em = nullptr;
      EntityRenderer* er = nullptr;
      agl::Sprite2D* terrain;
      agl::Sprite2D* fuck;
      static constexpr size_t FTIMES_TO_STORE = 128;
      float ftimes[FTIMES_TO_STORE];
      size_t curr = 0;
      agl::FBOTexMS ft;
      void setMVP();
      std::unordered_map<uint32_t, size_t> entityIDsByPlayer;
      // Add players that have started to exist,
      // remove players that no longer exist and
      // update the positions of players.
      void readPlayersFromGS();
      friend class Client;
    };
  }
}