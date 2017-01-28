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

#include <stdint.h>
#include <string>
#include <vector>

namespace x801 {
  namespace game {
    class ClientWindow;
    struct ChatEntry {
      uint32_t playerID;
      std::string message;
      ChatEntry(uint32_t playerID, const std::string& message) :
          playerID(playerID), message(message) {}
    };
    class ChatWindow {
    public:
      void setParent(ClientWindow* window) {
        this->window = window;
      }
      void pushMessage(uint32_t playerID, const std::string& message);
      void render();
    private:
      std::vector<ChatEntry> entries;
      char yourMessage[256];
      bool shouldScrollToBottom = false;
      ClientWindow* window = nullptr;
    };
  }
}