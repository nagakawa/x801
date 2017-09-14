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

#include <memory>
#include <string>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <Texture.h>

namespace x801 {
  namespace game {
    class Patcher;
    class TextureView {
    public:
      TextureView(Patcher* underlying) : underlying(underlying) {}
      boost::optional<agl::Texture>
          getTextureTransient(const std::string& name);
      agl::Texture* getTexture(const std::string& name);
      void purge(const std::string& name);
    private:
      Patcher* underlying;
      mutable boost::shared_mutex mapMutex;
      std::unordered_map<std::string, agl::Texture> textures;
    };
    inline void bindTextureFromPointer(agl::Texture* t) {
      if (t != nullptr) t->bind();
      else glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}