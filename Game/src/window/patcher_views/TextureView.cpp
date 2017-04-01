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

using namespace x801::game;

std::shared_ptr<agl::Texture> x801::game::TextureView::getTexture(const std::string& name) {
  boost::shared_lock<boost::shared_mutex> guard(mapMutex);
  auto iterator = textures.find(name);
  if (iterator != textures.end())
    return iterator->second;
  underlying->fetchFile(name.c_str());
  uint32_t version, contentLength;
  uint8_t* contents = nullptr;
  bool status = underlying->getFileEntry(
    name.c_str(),
    version, contentLength, contents
  );
  if (status) {
    std::shared_ptr<agl::Texture> tex = std::make_shared<agl::Texture>(
      contents, (int) contentLength
    );
    textures[name] = std::move(tex);
    return textures[name];
  } else {
    std::cerr << "Requesting file " << name << "\n";
    underlying->requestFile(name.c_str());
    return nullptr;
  }
}