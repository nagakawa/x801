#include "window/ChatWindow.h"

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

#include <iostream>
#include <sstream>
#include <imgui.h>

void x801::game::ChatWindow::pushMessage(uint32_t playerID, const std::string& message) {
  entries[(start + messageCount) % ringSize] = ChatEntry(playerID, message);
  ++messageCount;
  if (messageCount > ringSize) {
    start += messageCount - ringSize;
    messageCount = ringSize;
  }
  shouldScrollToBottom = true;
}

// Use technique in imgui_demo.cpp
void x801::game::ChatWindow::render() {
  ImGui::SetNextWindowSize(ImVec2(300, 800), ImGuiSetCond_FirstUseEver);
  bool isChatWindowOpen = ImGui::Begin("Chat");
  if (isChatWindowOpen) {
    ImGui::BeginChild(
      "ScrollingRegion",
      ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()),
      false,
      0 // ImGuiWindowFlags_HorizontalScrollbar
    );
    for (size_t i = 0; i < messageCount; ++i) {
      ChatEntry entry = entries[(start + i) % ringSize];
      std::stringstream output;
      output << "[" <<
        window->c->getUsername(entry.playerID) <<
        "] " << entry.message;
      std::string str = output.str();
      ImGui::TextWrapped("%s", str.c_str());
    }
    if (shouldScrollToBottom) {
      ImGui::SetScrollHere();
      shouldScrollToBottom = false;
    }
    ImGui::EndChild();
    ImGui::Separator();
    bool hasDone = ImGui::InputText(
      "Input", yourMessage, 256,
      ImGuiInputTextFlags_EnterReturnsTrue
    );
    if (hasDone && yourMessage[0] != '\0') {
      window->getParentClient()->sendChatMessage(yourMessage);
      yourMessage[0] = '\0';
    }
    if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
      ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
  }
  ImGui::End();
}