#include "window/Launcher.h"

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

#ifdef __unix__
#include <signal.h>
#endif
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <utils.h>
#include "window/imgui_hooks.h"

extern agl::GLFWApplication* agl::currentApp;

void x801::game::Launcher::initialise() {
  std::cerr << "x801::game::Launcher::initialise();\n";
  glfwSetInputMode(underlying(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(underlying(), x801::game::customKeyCallback);
  glfwSetMouseButtonCallback(underlying(), ImGui_ImplGlfwGL3_MouseButtonCallback);
  glfwSetScrollCallback(underlying(), ImGui_ImplGlfwGL3_ScrollCallback);
  glfwSetCharCallback(underlying(), ImGui_ImplGlfwGL3_CharCallback);
  ImGui_ImplGlfwGL3_Init(underlying(), false);
  ImGuiIO& io = ImGui::GetIO();
  //io.Fonts->AddFontFromFileTTF("/home/uruwi/kiloji/kiloji_p.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  io.Fonts->AddFontFromFileTTF("intrinsic-assets/VLGothic/VL-PGothic-Regular.ttf", 18.0f, nullptr, x801::game::range);
  currentEXE = x801::base::getPathOfCurrentExecutable();
  // blah blah blah
}

void x801::game::Launcher::tick() {
  if (glfwWindowShouldClose(underlying())) {
    glfwSetWindowShouldClose(underlying(), true);
  }
  ImGui_ImplGlfwGL3_NewFrame();
  glClearColor(1.0f, 0.8f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ImGui::Begin("Experiment801");
  if (ImGui::Button("Client")) tabno = 0;
  ImGui::SameLine();
  if (ImGui::Button("Server")) tabno = 1;
  ImGui::SameLine();
  if (ImGui::Button("Filehost")) tabno = 2;
  ImGui::Separator();
  body();
  ImGui::End();
  ImGui::Render();
}

void x801::game::Launcher::body() {
  switch (tabno) {
    case 0: {
      ImGui::Columns(2);
      ImGui::InputText("Address to connect", addressToConnect, 256);
      ImGui::InputInt("Port", &portToConnect);
      ImGui::InputText("Username", username, 256);
      ImGui::InputText("Password", password, 256,
        showPassword ? 0 : ImGuiInputTextFlags_Password);
      ImGui::Checkbox("Show password", &showPassword);
      if (client.id() > 0) {
        ImGui::Text("Process running (ID = %d)", client.id());
      } else {
        if (ImGui::Button("Login")) {
          // login
          client = boost::process::child(
            currentEXE,
            "-c", std::string(addressToConnect), std::to_string(portToConnect),
            "--username", std::string(username),
            "--password", std::string(password),
            boost::process::std_out > clientOutput
          );
          clientThread = std::move(feedThread(client, clientOutput, clientLog));
        }
      }
      ImGui::NextColumn();
      ImGui::TextWrapped("%s", clientLog.c_str());
      ImGui::Columns(1);
      break;
    }
    case 1: {
      break;
    }
    case 2: {
      break;
    }
  }
}

void x801::game::Launcher::readKeys() {

}

void x801::game::Launcher::onMouse(double xpos, double ypos) {
  (void) xpos; (void) ypos;
}

x801::game::Launcher::~Launcher() {
  ImGui_ImplGlfwGL3_Shutdown();
  if (clientThread.joinable()) clientThread.join();
}

void x801::game::Launcher::feed(
    boost::process::child& p,
    boost::process::ipstream& out,
    std::string& log) {
  std::string line;
  while (p.running() && std::getline(out, line)) {
    log += line;
    log += '\n';
  }
  // p.wait();
  p = boost::process::child();
}

std::thread x801::game::Launcher::feedThread(
    boost::process::child& p,
    boost::process::ipstream& out,
    std::string& log) {
  return std::thread([this, &p, &out, &log]() { this->feed(p, out, log); });
}