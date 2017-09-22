#include "Model.h"

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

#include "mapErrors.h"

namespace x801 {
  namespace map {
    const char* IllegalMap::mapErrorStrs[] = {
      "OK",
      "Redundant tilesec",
      "Decompression error",
      "Checksums mismatched",
      "Uncompressed size didn't match actual",
      "Unrecognised section",
      "XMap magic number didn't match",
      "Redundant section",
      "Missing essential section"
    };
  }
}