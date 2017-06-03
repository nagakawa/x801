#include "window/imgui_hooks.h"

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

void x801::game::customKeyCallback(
    GLFWwindow* window, int key, int scancode, int action, int mode) {
  ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mode);
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) return;
  if (action == GLFW_PRESS) agl::currentApp->setKey(key);
	else if (action == GLFW_RELEASE) agl::currentApp->resetKey(key);
}

const ImWchar x801::game::range[] = { 0x20, 0xFFFF, 0 };