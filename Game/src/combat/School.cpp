#include "combat/School.h"

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
    std::vector<School> defaultSchools = {
      // The following three schools are almost identical
      // to those of Wizard101.
      School("Fire", "Pyromancer", 403, 22),
      School("Ice", "Thaumaturge", 469, 31),
      School("Lightning", "Diviner", 384, 16),
      // The base health is based on Wizard101's Balance.
      School("Water", "Watersomething?", 453, 27),
      // The base health isn't really based on anything.
      School("Earth", "Earthsomething?", 423, 22),
      // Wind subjects gain incredible multi-hit abilities
      // at the cost of low health and attack.
      School("Wind", "Windsomething?", 395, 19),
      // The base health is based on Wizard101's Life.
      School("Light", "Luminary", 443, 17),
      // The base health is based on Wizard101's Death.
      School("Darkness", "Tenebromancer", 426, 24),
      // The following schools are used only for spells.
      School("Top", "###", 0, 0, false),
      School("Bottom", "###", 0, 0, false),
    };
  }
}