#include "window/entity_rendering/OverheadName.h"

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

namespace x801 {
  namespace game {
    OverheadName OverheadName::operator=(const OverheadName& other) {
      title = other.title;
      name = other.name;
      rank = other.rank;
      classifier = other.classifier;
      return *this;
    }
    OverheadName OverheadName::operator=(OverheadName&& other) {
      title = std::move(other.title);
      name = std::move(other.name);
      rank = other.rank;
      classifier = other.classifier;
      return *this;
    }
    std::string OverheadName::format() const {
      std::string s = "";
      if (title.length() != 0) {
        s += title;
        s += '\n';
      }
      s += name;
      if (classifier >= EntityClassifier::MOB_REGULAR1) {
        s += "\nRank ";
        s += std::to_string(rank);
        if (classifier == EntityClassifier::MOB_ELITE)
          s += " Elite";
        else if (classifier == EntityClassifier::MOB_BOSS)
          s += " Boss";
      }
      return s;
    }
    glm::vec4 OVERHEAD_COLOURS[] = {
      glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
      glm::vec4(0.2f, 0.8f, 1.0f, 1.0f),
      glm::vec4(0.2f, 1.0f, 0.2f, 1.0f),
      glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
      glm::vec4(1.0f, 0.8f, 0.2f, 1.0f),
      glm::vec4(1.0f, 0.2f, 0.2f, 1.0f),
      glm::vec4(0.8f, 0.4f, 0.8f, 1.0f),
    };
    glm::vec4 OverheadName::colour() const {
      return OVERHEAD_COLOURS[(uint16_t) classifier];
    }
  }
}