#include "window/patcher_views/MapView.h"

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
#include <sstream>

#include "window/Patcher.h"

using namespace x801::game;

std::shared_ptr<x801::map::Area> x801::game::MapView::getArea(
    const x801::map::QualifiedAreaID& id) {
  mapMutex.lock_shared();
  auto iterator = areas.find(id);
  if (iterator != areas.end()) {
    mapMutex.unlock_shared();
    return iterator->second;
  }
  std::string name =
    std::string("maps/map.") +
    std::to_string(id.worldID) + "." +
    std::to_string(id.areaID) + ".map";
  underlying->fetchFile(name.c_str());
  uint32_t version, contentLength;
  uint8_t* contents = nullptr;
  bool status = underlying->getFileEntry(
    name.c_str(),
    version, contentLength, contents
  );
  mapMutex.unlock_shared();
  if (status) {
    boost::unique_lock<boost::shared_mutex> guard(mapMutex);
    setMutex.lock();
    requestedAreas.erase(id);
    setMutex.unlock();
    std::string s((char*) contents, contentLength);
    std::stringstream in(s);
    std::shared_ptr<x801::map::Area> area =
      std::make_shared<x801::map::Area>(in);
    areas[id] = std::move(area);
    return areas[id];
  } else {
    //std::cout << "Failed to get map file " << name << "\n";
    setMutex.lock();
    if (requestedAreas.count(id) != 0) {
      setMutex.unlock();
      return nullptr;
    }
    requestedAreas.insert(id);
    setMutex.unlock();
    std::cerr << "Requesting file " << name << "\n";
    underlying->requestFile(name.c_str());
    return nullptr;
  }
}