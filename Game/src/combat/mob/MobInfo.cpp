#include "combat/mob/MobInfo.h"

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

#include <utils.h>

namespace x801 {
  namespace game {
    MobInfo::MobInfo(std::istream& fh) {
      using namespace x801::base;
      rank = readInt<uint16_t>(fh);
      type = readInt<uint16_t>(fh);
      id = readString<uint16_t>(fh);
      dispname = readString<uint16_t>(fh);
      title = readString<uint16_t>(fh);
      texname = readString<uint16_t>(fh);
      stats = Stats(fh);
    }
  }
}