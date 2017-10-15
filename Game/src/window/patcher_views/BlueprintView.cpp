#include "window/patcher_views/BlueprintView.h"

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

const x801::map::Blueprint*
x801::game::BlueprintView::getBlueprint(const std::string& name) {
  mapMutex.lock_shared();
  auto iterator = blueprints.find(name);
  if (iterator != blueprints.end()) {
    mapMutex.unlock_shared();
    return &(iterator->second);
  }
  std::string fullname = "assets/entity-models/blueprints/" + name + ".ebp";
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
    blueprints.emplace(name, input);
    // This is equivalent to &(blueprints[name])
    // but doesn't require Blueprint to have a default ctor.
    return &(blueprints.find(name)->second);
  } else {
    std::cerr << "Requesting file " << name << "\n";
    underlying->requestFile(name.c_str());
    return nullptr;
  }
}