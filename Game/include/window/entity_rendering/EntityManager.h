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
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>

#include "window/entity_rendering/Entity.h"

namespace x801 {
  namespace game {
    class EntityManager {
    public:
      EntityManager() : firstVacant(0) {}
      template<class E, class... Args>
      size_t addEntity(Args&&... args) {
        size_t id = firstVacant++;
        std::unique_ptr<Entity> e =
          std::make_unique<E>(std::forward<Args>(args)...);
        entityMutex.lock();
        entities[id] = std::move(e);
        entityMutex.unlock();
        return id;
      }
      Entity* getEntity(size_t id) {
        entityMutex.lock_shared();
        auto it = entities.find(id);
        if (it == entities.end()) {
          entityMutex.unlock_shared();
          return nullptr;
        }
        Entity* e = &*(it->second);
        entityMutex.unlock_shared();
        return e;
      }
      void deleteEntity(size_t id) {
        entityMutex.lock();
        entities.erase(id);
        entityMutex.unlock();
      }
      void forEach(std::function<void(Entity&)> cb);
    private:
      mutable boost::shared_mutex entityMutex;
      std::unordered_map<size_t, std::unique_ptr<Entity>> entities;
      std::atomic<size_t> firstVacant;
    };
  }
}