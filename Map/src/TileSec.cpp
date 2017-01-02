#include "TileSec.h"

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

using namespace x801::map;

#include <utils.h>

#pragma GCC diagnostic push                // we DO want an explicit ctor
#pragma GCC diagnostic ignored "-Weffc++"  // since it has complex behaviour
x801::map::TileSec::TileSec(std::istream& fh) {
  layerCount = x801::base::readInt<uint16_t>(fh);
  for (int i = 0; i < layerCount; ++i) {
    layers.emplace_back(fh);
  }
}
#pragma GCC diagnostic pop

void x801::map::TileSec::write(std::ostream& fh) const {
  x801::base::writeInt<uint16_t>(fh, layerCount);
  for (int i = 0; i < layerCount; ++i) {
    layers[i].write(fh);
  }
}

TileSec& x801::map::TileSec::operator=(const TileSec& that) {
  layerCount = that.layerCount;
  layers = that.layers;
  return *this;
}
