#include "combat/Stats.h"

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

#include <algorithm>

namespace x801 {
  namespace game {
    size_t baseMana(size_t level) {
      return 15 + 2 * level;
    }
    size_t basePowerChance(size_t level) {
      if (level < 10) return 0;
      if (level >= 40) return 40;
      return level;
    }
    Stats::Stats(const StatsUser& su, const std::vector<School>& schools) {
      level = su.level;
      school = su.school;
      maxHealth = schools[school].getBaseHealth(level);
      maxMana = baseMana(level);
      powerPipChance = basePowerChance(level);
    }
  }
}