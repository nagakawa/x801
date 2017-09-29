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
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/thread/shared_mutex.hpp>

#include "combat/mob/MobInfo.h"

namespace x801 {
  namespace game {
    class Patcher;
    class MobInfoView {
    public:
      MobInfoView(Patcher* underlying) : underlying(underlying) {}
      const MobInfo* getInfo(const std::string& nae);
    private:
      Patcher* underlying;
      mutable boost::shared_mutex mapMutex;
      std::unordered_map<std::string, std::unique_ptr<MobInfo>> infos;
    };
  }
}