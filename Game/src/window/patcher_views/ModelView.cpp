#include "window/patcher_views/ModelView.h"

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
#include <string>

#include "window/Patcher.h"

using namespace x801::game;

x801::map::ModelApplicationIndex* x801::game::ModelView::getMAI() {
  std::lock_guard<std::mutex> guard(m1);
  if (mai != nullptr) return mai;
  // we don't have the MAI yet; get the file and read it
  underlying->fetchFile("models/all.mai");
  uint32_t version, contentLength;
  uint8_t* contents = nullptr;
  bool status = underlying->getFileEntry(
    "models/all.mai",
    version, contentLength, contents
  );
  if (status) {
    std::stringstream in(std::string((char*) contents, contentLength));
    mai = new x801::map::ModelApplicationIndex(in);
    return mai;
  } else {
    std::cerr << "Requesting file models/all.mai\n";
    underlying->requestFile("models/all.mai");
    return nullptr;
  }
}

x801::map::ModelFunctionIndex* x801::game::ModelView::getMFI() {
  std::lock_guard<std::mutex> guard(m2);
  if (mfi != nullptr) return mfi;
  // we don't have the MFI yet; get the file and read it
  underlying->fetchFile("models/all.mfi");
  uint32_t version, contentLength;
  uint8_t* contents = nullptr;
  bool status = underlying->getFileEntry(
    "models/all.mfi",
    version, contentLength, contents
  );
  if (status) {
    std::stringstream in(std::string((char*) contents, contentLength));
    mfi = new x801::map::ModelFunctionIndex(in);
    return mfi;
  } else {
    std::cerr << "Requesting file models/all.mfi\n";
    underlying->requestFile("models/all.mfi");
    return nullptr;
  }
}
