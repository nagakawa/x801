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
#include <utility>

#include <utils.h>

namespace x801 {
  namespace game {
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
    }
    MobManager::MobManager(x801::map::PathSec&& ps) {
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
          std::forward_as_tuple(box, std::move(p)));
      }
    }
  }
}