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
#include <string>
#include <thread>
#include <GLFWApplication.h>
#include <imgui.h>
#include <boost/process.hpp>

namespace x801 {
  namespace game {
    class Launcher : public agl::GLFWApplication {
    public:
      using agl::GLFWApplication::GLFWApplication;
      void initialise();
      void tick();
      void readKeys();
      void onMouse(double xpos, double ypos);
      void start() {
        GLFWApplication::start();
      }
      ~Launcher();
    private:
      int tabno = 0;
      char addressToConnect[256] = "";
      int portToConnect = 9001;
      char username[256] = "";
      char password[256] = "";
      bool showPassword = false;
      std::string currentEXE;
      std::string clientLog;
      boost::process::child client;
      boost::process::ipstream clientOutput;
      std::thread clientThread;
      void feed(
        boost::process::child& p,
        boost::process::ipstream& out,
        std::string& log);
      std::thread feedThread(
        boost::process::child& p,
        boost::process::ipstream& out,
        std::string& log);
      void body();
    };
  }
}