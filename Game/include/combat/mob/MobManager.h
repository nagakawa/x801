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

#include <atomic>
#include <functional>
#include <memory>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <boost/random/random_device.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <zekku/QuadTree.h>
#include <PathSec.h>

#include "combat/mob/Mob.h"

namespace x801 {
  namespace game {
    class GameState;
    class MobManager;
    // A server-side instantiation of a mob path.
    class MobPath {
    public:
      MobPath(
        zekku::AABB<float> bounds,
        x801::map::Path&& path,
        MobManager* manager) :
            mobs(bounds), path(std::move(path)),
            manager(manager) {
          weightSum = 0;
          size_t size = this->path.weights.size();
          weightCumul = std::vector<uint32_t>(size);
          for (size_t i = 0; i < size; ++i) {
            weightCumul[i] = weightSum;
            weightSum += this->path.weights[i];
          }
          boost::random::random_device rd;
          unsigned int seed = rd();
          gen.seed(seed);
        }
      /*
      template<class E, class... Args>
      size_t addEntity(Args&&... args) {
        static_assert(std::is_base_of<Entity, E>::value,
          "Trying to add something that isn't an Entity");
        size_t id = firstVacant++;
        std::unique_ptr<Entity> e =
          std::make_unique<E>(std::forward<Args>(args)...);
        entityMutex.lock();
        names[id] = e->overheadName();
        entities[id] = std::move(e);
        entityMutex.unlock();
        return id;
      }
      */
      Mob& getEntity(zekku::Handle<> id);
      void deleteEntity(zekku::Handle<> id);
      void forEach(std::function<void(Mob&)> cb);
      // void forEachOver(std::function<void(Mob&, OverheadName&)> cb);
      void advanceFrame(float s);
      zekku::QuadTree<Mob,
        uint16_t, float, zekku::QUADTREE_NODE_COUNT,
        MobGetXY>& getMobs() { return mobs; }
      const x801::map::Path& getPath() const { return path; }
    private:
      mutable boost::shared_mutex entityMutex;
      zekku::QuadTree<Mob,
        uint16_t, float, zekku::QUADTREE_NODE_COUNT,
        MobGetXY> mobs;
      x801::map::Path path;
      MobManager* manager;
      float timeLeft = 0.0f;
      uint32_t weightSum;
      std::vector<uint32_t> weightCumul;
      size_t randomMob();
      std::mt19937 gen;
    };
    class MobManager {
    public:
      MobManager(x801::map::PathSec&& ps, GameState* gs);
      void advanceFrame(float s);
      using PI = std::unordered_multimap<int, MobPath>::iterator;
      std::pair<PI, PI> getPathRange(int z) {
        return paths.equal_range(z);
      }
      PI pathEnd() { return paths.end(); }
    private:
      std::unordered_multimap<int, MobPath> paths;
      GameState* gs;
      friend class MobPath;
    };
  }
}