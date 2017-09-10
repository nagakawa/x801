#include "pixelratio.h"

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

#include <math.h>
#include <algorithm>

namespace x801 {
  namespace base {
    size_t calculatePixelScale(
      size_t tileSize,
      size_t screenWidth,
      size_t screenHeight,
      size_t desiredArea
    ) {
      // Calculate how many tiles are visible
      // given a scale of 1
      double actualArea = screenWidth * screenHeight;
      actualArea /= tileSize * tileSize;
      double ratio = actualArea / desiredArea;
      if (ratio < 1) return 1;
      return (size_t) sqrt(ratio);
    }
  }
}