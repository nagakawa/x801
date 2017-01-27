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

void x801::game::ClientWindow::initialise() {
  std::cerr << "x801::game::ClientWindow::initialise();\n";
  glfwSetInputMode(underlying(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void x801::game::ClientWindow::tick() {
  if (c->isDone() || glfwWindowShouldClose(underlying())) {
    glfwSetWindowShouldClose(underlying(), true);
  }
}

void x801::game::ClientWindow::readKeys() {

}

void x801::game::ClientWindow::onMouse(double xpos, double ypos) {
  (void) xpos; (void) ypos;
}

x801::game::ClientWindow::~ClientWindow() {

}