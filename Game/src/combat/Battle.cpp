#include "combat/Battle.h"

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

#include "packet.h"
#include "combat/Stats.h"

namespace x801 {
  namespace game {
    void Battle::damage(
        size_t attacker, size_t defender,
        size_t school, const mpz_class& amt,
        RakNet::BitStream& out, size_t& nPacts) {
      // TODO handle ALL_ENEMIES and ALL_ALLIES for defender
      if (defender == ALL_ENEMIES || defender == ALL_ALLIES) {
        throw "x801::game::Battle::damage: ALL_ENEMIES and ALL_ALLIES nyi";
      }
      Entity& a = players[attacker];
      Entity& d = players[defender];
      mpz_class effectiveAmt = (a.stats->getDamage(school) + 1000) * amt;
      effectiveAmt /= (d.stats->getResist(school) + 1000);
      // TODO: this should account for hanging effects when they're added
      out.Write((uint16_t) PactTypes::damage);
      out.Write((uint8_t) defender);
      writeMPZToBitStream(out, effectiveAmt);
      ++nPacts;
      // Now update the battle
      d.health -= effectiveAmt;
      if (sgn(d.health) < 0) d.health = 0;
    }
  }
}