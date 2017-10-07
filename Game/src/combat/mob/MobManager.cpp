#include "combat/mob/MobManager.h"

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

#include <assert.h>
#include <algorithm>
#include <utility>

#include <utils.h>

#include "GameState.h"

namespace x801 {
  namespace game {
    static constexpr float SPAWN_DELAY = 2.5f;
    Mob& MobPath::getEntity(zekku::Handle<> id) {
      return mobs.deref(id);
    }
    void MobPath::deleteEntity(zekku::Handle<> id) {
      mobs.deref(id).marked = true;
    }
    void MobPath::forEach(std::function<void(Mob&)> cb) {
      mobs.querym(zekku::QueryAll<float>(), [cb](Mob& mob) {
        cb(mob);
      });
    }
    void MobPath::advanceFrame(float s) {
      auto mapcond = [this, s](const Mob& m) {
        Mob m2 = m;
        m2.advanceFrame(s, path);
        return m2;
      };
      auto filtcond = [this](const Mob& m) {
        return m.progress < path.vertices.size();
      };
      zekku::QuadTree<Mob,
        uint16_t, float, zekku::QUADTREE_NODE_COUNT,
        MobGetXY> mobs2 = mobs.mapIf(mapcond, filtcond);
      mobs = std::move(mobs2);
      timeLeft -= s;
      if (timeLeft <= 0) {
        timeLeft += SPAWN_DELAY;
        std::cout << "Mobs should spawn NOW!\n";
        size_t index = randomMob();
        Mob m;
        m.pos = { path.vertices[0].x, path.vertices[0].y };
        m.progress = 0.0f;
        m.info = manager->gs->getMobInfo(path.mobNames[index]);
        mobs.insert(m);
      }
    }
    size_t MobPath::randomMob() {
      std::uniform_int_distribution<uint32_t> dist(0, weightSum - 1);
      uint32_t value = dist(gen);
      auto it = std::upper_bound(
        weightCumul.begin(), weightCumul.end(), value);
      // it now points one element after the one we want
      return it - weightCumul.begin() - 1;
    }
    MobManager::MobManager(x801::map::PathSec&& ps, GameState* gs) {
      // I will `suck` the paths out of this section
      // Cssssssssssssssssssssssssssssssss
      for (x801::map::Path& p : ps.paths) {
        zekku::AABB<float> box = {
          {(p.maxX + p.minX) / 2.0, (p.maxY + p.minY) / 2.0},
          {(p.maxX - p.minX) / 2.0, (p.maxY - p.minY) / 2.0}
        };
        int z = p.z;
        paths.emplace(std::piecewise_construct,
          std::forward_as_tuple(z),
          std::forward_as_tuple(box, std::move(p), this));
      }
      this->gs = gs;
    }
    void MobManager::advanceFrame(float s) {
      std::cout << "bepis " << paths.size() << "\n";
      for (auto& p : paths) {
        p.second.advanceFrame(s);
      }
    }
  }
}