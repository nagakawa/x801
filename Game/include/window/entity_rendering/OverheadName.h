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

namespace x801 {
  namespace game {
    enum class EntityClassifier : uint16_t {
      PLAYER,
      NPC,
      MOB_REGULAR1, // yellow name (as in w101)
      MOB_REGULAR2, // orange name
      MOB_ELITE, // red name
      MOB_BOSS, // purple name
    };
    class OverheadName {
    public:
      OverheadName(
          std::string title,
          std::string name,
          uint16_t rank,
          EntityClassifier classifier) :
        title(title), name(name), rank(rank), classifier(classifier) {}
      OverheadName(const OverheadName& other) :
        title(other.title), name(other.name),
        rank(other.rank), classifier(other.classifier) {}
      OverheadName(OverheadName&& other) :
        title(std::move(other.title)), name(std::move(other.name)),
        rank(other.rank), classifier(other.classifier) {}
      OverheadName operator=(const OverheadName& other);
      OverheadName operator=(OverheadName&& other);
      std::string title;
      std::string name;
      uint16_t rank;
      EntityClassifier classifier;
    };
  }
}