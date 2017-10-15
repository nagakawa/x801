#include "XDatSec.h"

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

namespace x801 {
  namespace map {
    XDatSec::XDatSec(std::istream& fh) {
      worldName = x801::base::readString<uint16_t>(fh);
      areaName = x801::base::readString<uint16_t>(fh);
      for (size_t i = 0; i < 3; ++i) {
        bgColour[i] = (uint8_t) fh.get();
      }
      present = true;
    }
    void XDatSec::write(std::ostream& fh) const {
      x801::base::writeString<uint16_t>(fh, worldName);
      x801::base::writeString<uint16_t>(fh, areaName);
      for (size_t i = 0; i < 3; ++i) {
        fh.put((char) bgColour[i]);
      }
    }
  }
}