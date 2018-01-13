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

#include "GameState.h"
#include "Player.h"
#include "packet.h"
#include "combat/Stats.h"
#include "combat/mob/Mob.h"
#include "combat/mob/MobInfo.h"

namespace x801 {
  namespace game {
    Battle::Battle(glm::vec2 xy, uint32_t id) :
        position(xy), id(id) {}
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
    void Battle::Entity::read(Mob& m) {
      m.marked = true;
      mobInfo = m.info;
      stats = &(mobInfo->stats);
      health = stats->maxHealth;
      mana = stats->maxMana;
    }
    // XXX: does not set battle ID of player
    void Battle::Entity::read(Player& p) {
      assert(p.getBattleID() == 0);
      playerID = p.getPlayerID();
      stats = &(p.getStats());
      health = stats->maxHealth; // TODO save player health
      mana = stats->maxMana;     // and mana
    }
    void Battle::slot(Mob& m, size_t index) {
      assert(index >= 0 && index < 2 * PLAYERS_PER_SIDE);
      assert(players[index].stats == nullptr);
      players[index].read(m);
    }
    void Battle::slot(Player& p, size_t index) {
      assert(index >= 0 && index < 2 * PLAYERS_PER_SIDE);
      assert(players[index].stats == nullptr);
      players[index].read(p);
      p.setBattleID(id);
    }
    BattleManager::BattleManager(AreaWithPlayers* a) :
      battles({{0, 0}, {2048, 2048}}), a(a), globalID(0) {}
    BattleManager::Handle BattleManager::spawnBattle(glm::vec2 xy) {
      boost::shared_lock<boost::shared_mutex> guard(lock);
      do ++globalID;
      while (battlesByID.count(globalID) != 0);
      uint32_t id = globalID;
      std::unique_ptr<Battle> battle = std::make_unique<Battle>(xy, id);
      battlesByID[id] = battle.get();
      return battles.insert(std::move(battle));
    }
  }
}