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
#include <unordered_map>
#include <vector>

#include <Texture.h>

#include <Chunk.h>

#include "Location.h"
#include "window/entity_rendering/OverheadName.h"

namespace x801 {
  namespace game {
    class ClientWindow;
    class Entity {
    public:
      virtual ~Entity() {};
      virtual void advanceFrame() = 0;
      virtual size_t getTexture() = 0;
      virtual Location getLocation() = 0;
      virtual bool setLocation(const Location& l) = 0;
      virtual OverheadName overheadName() {
        return OverheadName();
      }
      virtual bool isPlayer() { return false; }
    protected:
      static x801::map::EntityTextureBindings* tb;
      friend class EntityRenderer;
    };
    class PlayerEntity : public Entity {
    public:
      PlayerEntity(uint32_t id, Location l) :
        id(id), l(l), walkFrame(0) {}
      ~PlayerEntity() override {}
      void advanceFrame() override { walkFrame += 1; }
      size_t getTexture() override;
      Location getLocation() override { return l; }
      bool setLocation(const Location& l) override {
        this->l = l;
        return true;
      }
      OverheadName overheadName() override;
      bool isPlayer() override { return true; }
    private:
      uint32_t id;
      Location l;
      size_t walkFrame;
      static ClientWindow* cw;
      friend class ClientWindow;
    };
  }
}
