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
namespace x801 {
  namespace game {
    class ClientWindow;
  }
}
#include "Client.h"
#include "window/ChatWindow.h"
#include "window/TerrainRenderer.h"

namespace x801 {
  namespace game {
    class ClientWindow : public agl::GLFWApplication {
    public:
      using agl::GLFWApplication::GLFWApplication;
      void initialise();
      void tick();
      void readKeys();
      void onMouse(double xpos, double ypos);
      void start() {
        GLFWApplication::start();
      }
      ChatWindow* getChatWindow() { return chat; }
      Client* getParentClient() { return c; }
      virtual ~ClientWindow() override;
      Client* c;
    private:
      ChatWindow* chat = nullptr;
      TerrainRenderer* tr = nullptr;
      agl::Sprite2D* terrain;
      agl::Sprite2D* fuck;
      static constexpr size_t FTIMES_TO_STORE = 128;
      float ftimes[FTIMES_TO_STORE];
      size_t curr = 0;
      agl::FBOTexMS ft;
    };
  }
}