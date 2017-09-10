#include "window/patcher_views/TextureView.h"

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

#include <iostream>

namespace x801 {
  namespace game {
    static agl::TexInitInfo opts = {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true, false, false};
    boost::optional<agl::Texture>
        TextureView::getTextureTransient(const std::string& name) {
      std::cerr << "Getting texture " << name << "\n";
      underlying->fetchFile(name.c_str());
      uint32_t version, contentLength;
      uint8_t* contents = nullptr;
      bool status = underlying->getFileEntry(
        name.c_str(),
        version, contentLength, contents
      );
      if (status)
        return agl::Texture(contents, (int) contentLength, opts);
      return boost::none;
    }
    agl::Texture* TextureView::getTexture(const std::string& name) {
      mapMutex.lock_shared();
      auto iterator = textures.find(name);
      if (iterator != textures.end()) {
        mapMutex.unlock_shared();
        return &(iterator->second);
      }
      mapMutex.unlock_shared();
      auto texopt = getTextureTransient(name);
      if (texopt) {
        boost::unique_lock<boost::shared_mutex> guard(mapMutex);
        textures[name] = std::move(*texopt);
        return &(textures[name]);
      } else {
        std::cerr << "Requesting file " << name << "\n";
        underlying->requestFile(name.c_str());
        return nullptr;
      }
    }
    void TextureView::purge(const std::string& name) {
      textures.erase(name);
    }
  }
}
