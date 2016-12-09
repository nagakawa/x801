#include "Area.h"

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

#include <stdlib.h>
#include <zlib.h>
#include <sstream>
#include <string>
#include <utils.h>
#include "mapErrors.h"

x801::map::Area::Area(std::istream& fh) {
  int header = x801::base::readInt<uint32_t>(fh);
  if (header != 0x70614d58) { // XMap
    error = MAPERR_NOT_A_MAP;
  }
  version = x801::base::Version(fh);
  worldID = x801::base::readInt<uint16_t>(fh);
  areaID = x801::base::readInt<uint16_t>(fh);
  uint32_t dataSectionCount = x801::base::readInt<uint32_t>(fh);
  for (unsigned int i = 0; i < dataSectionCount; ++i) {
    readSection(fh);
  }
}

x801::map::Area::~Area() {
  if (ts != nullptr) delete ts;
}

#define SECTION_TILE 0x454c4954L

int x801::map::Area::readSection(std::istream& fh) {
  int stat = MAPERR_OK;
  uint32_t sectionID = x801::base::readInt<uint32_t>(fh);
  uint32_t size = x801::base::readInt<uint32_t>(fh);
  bool isCompressed = size >> 31;
  size &= 0x7fffffffL;
  uint32_t uncompressedSize =
    isCompressed ? x801::base::readInt<uint32_t>(fh) : size;
  uint32_t adler32Expected = x801::base::readInt<uint32_t>(fh);
  int startPos = fh.tellg();
  char* buffer = new char[256];
  uint32_t adler32Actual = adler32(0L, Z_NULL, 0);
  for (unsigned int i = 0; i < (size >> 8); ++i) {
    fh.read(buffer, 256);
    adler32Actual = adler32(adler32Actual, (const unsigned char*) buffer, 256);
  }
  fh.read(buffer, size & 255);
  adler32Actual = adler32(adler32Actual, (const unsigned char*) buffer, size & 255);
  delete[] buffer;
  if (!isCompressed) fh.seekg(startPos);
  if (adler32Expected != adler32Actual) return MAPERR_CHECKSUM_MISMATCH;
  std::istream* input;
  if (isCompressed) {
    char* data = nullptr;
    uint32_t amtReadC;
    uint32_t amtReadU;
    x801::base::readZipped(fh, data, amtReadC, amtReadU);
    if (amtReadC != size || amtReadU != uncompressedSize) {
      stat = MAPERR_WRONG_SIZE;
      free(data);
      goto cleanup;
    }
    std::string s(data, uncompressedSize);
    input = new std::stringstream(s, std::ios_base::in | std::ios_base::binary);
    free(data);
  } else {
    input = &fh;
  }
  switch (sectionID) {
    case SECTION_TILE: {
      if (ts != nullptr) {
        stat = MAPERR_REDUNDANT_TILESEC;
        goto cleanup;
      }
      ts = new TileSec(*input);
      break;
    }
    default: {
      // 0b11100000 | 0b01000000
      // i. e. check for capital first letter
      if ((sectionID & 0xe0) == 0x40) {
        stat = MAPERR_UNRECOGNISED_SECTION;
        goto cleanup;
      }
    }
  }
  cleanup:
  if (isCompressed) delete input;
  return stat;
}
