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
#include <type_traits>
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>

#include "window/entity_rendering/Entity.h"
#include "window/entity_rendering/EntityRenderer.h"
#include "window/entity_rendering/OverheadName.h"

namespace x801 {
  namespace game {
    class ClientWindow;
    class EntityManager {
    public:
      EntityManager(ClientWindow* cw, agl::FBOTexMS& ft) :
        er(cw, ft, this), firstVacant(0) {}
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
      Entity* getEntity(size_t id);
      void deleteEntity(size_t id);
      void forEach(std::function<void(Entity&)> cb);
      void forEachOver(std::function<void(Entity&, OverheadName&)> cb);
      void advanceFrame();
      void updateUsernames();
      EntityRenderer er;
    private:
      mutable boost::shared_mutex entityMutex;
      std::unordered_map<size_t, std::unique_ptr<Entity>> entities;
      std::unordered_map<size_t, OverheadName> names;
      std::atomic<size_t> firstVacant;
    };
  }
}