#include "window/entity_rendering/EntityManager.h"

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
    void EntityManager::forEach(std::function<void(Entity&)> cb) {
      entityMutex.lock_shared();
      for (auto& p : entities) {
        cb(*(p.second));
      }
      entityMutex.unlock_shared();
    }
    void EntityManager::deleteEntity(size_t id) {
      entityMutex.lock();
      entities.erase(id);
      entityMutex.unlock();
    }
    Entity* EntityManager::getEntity(size_t id) {
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
    void EntityManager::advanceFrame() {
      entityMutex.lock_shared();
      for (auto& p : entities) {
        p.second->advanceFrame();
      }
      entityMutex.unlock_shared();
    }
  }
}