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

#include <assert.h>
#include <stdlib.h>
#include <zlib.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <utils.h>
#include "mapErrors.h"

x801::map::Area::Area(std::istream& fh, bool dontCare) {
  error = 0;
  int header = x801::base::readInt<uint32_t>(fh);
  if (header != 0x70614d58L) { // XMap
    error = MAPERR_NOT_A_MAP;
    index = -1;
  }
  version = x801::base::Version(fh);
  id.worldID = x801::base::readInt<uint16_t>(fh);
  id.areaID = x801::base::readInt<uint16_t>(fh);
  uint32_t dataSectionCount = x801::base::readInt<uint32_t>(fh);
  for (unsigned int i = 0; i < dataSectionCount; ++i) {
    int res = readSection(fh, dontCare);
    if (res != MAPERR_OK) {
      index = i;
      error = res;
      break;
    }
  }
  if (error == 0 && (ts == nullptr || !xs.present))
    error = MAPERR_MISSING_SECTION;
}

void x801::map::Area::write(std::ostream& fh) const {
  x801::base::writeInt<uint32_t>(fh, 0x70614d58L);
  x801::base::engineVersion.write(fh);
  x801::base::writeInt<uint16_t>(fh, id.worldID);
  x801::base::writeInt<uint16_t>(fh, id.areaID);
  int pos = fh.tellp();
  x801::base::writeInt<uint32_t>(fh, 0);
  int ds = 0;
  writeTileSection(fh, ds);
  writeXDatSection(fh, ds);
  writePOISection(fh, ds);
  writePathSection(fh, ds);
  fh.seekp(pos);
  x801::base::writeInt<uint32_t>(fh, ds);
  fh.seekp(0, std::ios_base::end);
}

x801::map::Area::~Area() {
  delete ts;
}

#define SECTION_TILE 0x324c4954L // "TIL2"
#define SECTION_XDAT 0x54414458L // "XDAT"
#define SECTION_pOIS 0x53494F70L // "pOIS"
#define SECTION_pATH 0x48544170L // "pATH"

int x801::map::Area::readSection(std::istream& fh, bool dontCare) {
  int stat = MAPERR_OK;
  uint32_t sectionID = x801::base::readInt<uint32_t>(fh);
  uint32_t size = x801::base::readInt<uint32_t>(fh);
  bool isCompressed = size >> 31;
  size &= 0x7fffffffL;
  // std::cerr << "Size: " << size << "\n";
  uint32_t uncompressedSize =
    isCompressed ? x801::base::readInt<uint32_t>(fh) : size;
  uint32_t adler32Expected = x801::base::readInt<uint32_t>(fh);
  if (!dontCare) {
    int startPos = fh.tellg();
    char buffer[256];
    uint32_t adler32Actual = adler32(0L, Z_NULL, 0);
    for (unsigned int i = 0; i < (size >> 8); ++i) {
      fh.read(buffer, 256);
      adler32Actual = adler32(adler32Actual, (const unsigned char*) buffer, 256);
    }
    fh.read(buffer, size & 255);
    adler32Actual = adler32(adler32Actual, (const unsigned char*) buffer, size & 255);
    fh.clear();
    fh.seekg(startPos);
    assert(fh.tellg() == startPos);
    /*std::clog << std::hex << adler32Expected << " vs " <<
      adler32Actual << '\n' << std::dec;*/
    if (adler32Expected != adler32Actual)
      return MAPERR_CHECKSUM_MISMATCH;
  }
  std::istream* input = nullptr;
  if (isCompressed) {
    // long base = fh.tellg();
    char* data = nullptr;
    uint32_t amtReadC;
    uint32_t amtReadU;
    stat = x801::base::readZipped(fh, data, amtReadC, amtReadU);
    if (stat != MAPERR_OK) goto cleanup;
    if (amtReadC != size || amtReadU != uncompressedSize) {
      //fprintf(stderr, "%d vs %d; %d vs %d\n", amtReadC, size, amtReadU, uncompressedSize);
      stat = MAPERR_WRONG_SIZE;
      if (data != nullptr) free(data);
      goto cleanup;
    }
    std::string s(data, uncompressedSize);
    input = new std::stringstream(s, std::ios_base::in | std::ios_base::binary);
    free(data);
    // assert(fh.tellg() == base + size);
    //std::clog << dynamic_cast<std::stringstream*>(input)->str();
  } else {
    std::string s(size, '\0');
    fh.read(&(s[0]), size);
    // for (char c : s) std::cerr << (int) c << ' ';
    // std::cerr << '\n';
    input = new std::stringstream(s, std::ios_base::in | std::ios_base::binary);
  }
  switch (sectionID) {
#define CREATE_READ_HANDLER(id, check, field, Class, isnew) \
    case id: { \
      if (check) { \
        stat = MAPERR_REDUNDANT_TILESEC; \
        goto cleanup; \
      } \
      field = isnew Class(*input); \
      break; \
    }
    CREATE_READ_HANDLER(SECTION_TILE, ts != nullptr, ts, TileSec, new)
    CREATE_READ_HANDLER(SECTION_XDAT, xs.present, xs, XDatSec, )
    CREATE_READ_HANDLER(SECTION_pOIS, ps.present, ps, POISec, )
    CREATE_READ_HANDLER(SECTION_pATH, pas.present, pas, PathSec, )
    default: {
      // 0b11100000 | 0b01000000
      // i. e. check for capital first letter
      std::clog << "Unrecognised section ID " <<
        (char) (sectionID) <<
        (char) (sectionID >> 8) <<
        (char) (sectionID >> 16) <<
        (char) (sectionID >> 24) << '\n';
      if ((sectionID & 0xe0) == 0x40) {
        stat = MAPERR_UNRECOGNISED_SECTION;
        goto cleanup;
      }
    }
  }
  cleanup:
  delete input;
  return stat;
}

void x801::map::Area::writeSection(std::ostream& fh, uint32_t sectionID, const char* data, uint32_t len, int& ds) const {
  x801::base::writeInt<uint32_t>(fh, sectionID);
  std::stringstream output(std::ios_base::out | std::ios_base::binary);
  uint32_t amtWrittenC;
  int stat = x801::base::writeZipped(output, data, len, amtWrittenC);
  assert(stat == Z_OK);
  if (false) {
    char* uData = nullptr;
    uint32_t amtReadC;
    uint32_t amtReadU;
    std::string cs = output.str();
    output.clear();
    output.seekg(0);
    stat = x801::base::readZipped(output, uData, amtReadC, amtReadU);
    for (unsigned int i = 0; i < cs.length(); ++i) {
      std::cout << (int) (unsigned char) cs[i] << ' ';
    }
    std::cout << '\n' << std::string(uData, amtReadU) << '\n';
    std::cout << "Compressed size: " << amtReadC << '\n';
    std::cout << "Uncompressed size: " << amtReadU << '\n';
    std::cout << stat << '\n';
    free(uData);
  }
  uint32_t adler = adler32(0L, Z_NULL, 0);
  if (amtWrittenC < len) {
    // Write it compressed.
    x801::base::writeInt<uint32_t>(fh, 0x80000000UL | amtWrittenC);
    x801::base::writeInt<uint32_t>(fh, len);
    std::string s = output.str();
    adler = adler32(adler, (const unsigned char*) s.c_str(), amtWrittenC);
    x801::base::writeInt<uint32_t>(fh, adler);
    fh.write(s.c_str(), amtWrittenC);
  } else {
    // Write it uncompressed.
    x801::base::writeInt<uint32_t>(fh, len);
    adler = adler32(adler, (const unsigned char*) data, len);
    x801::base::writeInt<uint32_t>(fh, adler);
    fh.write(data, len);
  }
  ++ds;
}

#define CREATE_WRITE_METHOD(Class, field, check, fatal, d, secid) \
  void x801::map::Area::write##Class(std::ostream& fh, int& ds) const { \
    std::stringstream data; \
    if (check) { \
      CHECK_FAILED(Class, check, fatal); \
    } \
    field d write(data); \
    std::string s = data.str(); \
    writeSection(fh, secid, s.c_str(), s.length(), ds); \
  }
#define CHECK_FAILED(Class, check, fatal) \
  CHECK_FAILED_##fatal(Class, check)
#define CHECK_FAILED_true(Class, check) \
  throw "x801::map::Area::write" #Class ": " #check "?"
#define CHECK_FAILED_false(Class, check) return

CREATE_WRITE_METHOD(TileSection, ts, ts == nullptr, true, ->, SECTION_TILE)
CREATE_WRITE_METHOD(XDatSection, xs, !xs.present, true, ., SECTION_XDAT)
CREATE_WRITE_METHOD(POISection, ps, !ps.present, false, ., SECTION_pOIS)
CREATE_WRITE_METHOD(PathSection, pas, !pas.present, false, ., SECTION_pATH)
