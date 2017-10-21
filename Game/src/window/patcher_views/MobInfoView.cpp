#include "window/patcher_views/MobInfoView.h"

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

#include "window/Patcher.h"

namespace x801 {
  namespace game {
    const MobInfo* MobInfoView::getInfo(const std::string& name) {
      mapMutex.lock_shared();
      auto iterator = infos.find(name);
      if (iterator != infos.end()) {
        mapMutex.unlock_shared();
        return iterator->second.get();
      }
      std::string fullname = "mobs/" + name + ".mob";
      underlying->fetchFile(fullname.c_str());
      uint32_t version, contentLength;
      uint8_t* contents = nullptr;
      bool status = underlying->getFileEntry(
        fullname.c_str(),
        version, contentLength, contents
      );
      mapMutex.unlock_shared();
      if (status) {
        boost::unique_lock<boost::shared_mutex> guard(mapMutex);
        std::string inputStr((char*) contents, contentLength);
        std::stringstream input(inputStr);
        std::unique_ptr<MobInfo> mi = std::make_unique<MobInfo>(input);
        infos[name] = std::move(mi);
        // This is equivalent to &(infos[name])
        // but doesn't require MobInfo to have a default ctor.
        return infos.find(name)->second.get();
      } else {
        std::cerr << "Requesting file " << name << "\n";
        underlying->requestFile(fullname.c_str());
        return nullptr;
      }
    }
  }
}