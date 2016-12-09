#include "utils.h"

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

#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <string>

using namespace x801::base;

template<> uint16_t x801::base::convLER<uint16_t>(uint16_t x) { return le16toh(x); }
template<> uint32_t x801::base::convLER<uint32_t>(uint32_t x) { return le32toh(x); }
template<> uint64_t x801::base::convLER<uint64_t>(uint64_t x) { return le64toh(x); }
template<> uint16_t x801::base::convLEW<uint16_t>(uint16_t x) { return htole16(x); }
template<> uint32_t x801::base::convLEW<uint32_t>(uint32_t x) { return htole32(x); }
template<> uint64_t x801::base::convLEW<uint64_t>(uint64_t x) { return htole64(x); }
template<> int16_t x801::base::convLER<int16_t>(int16_t x) { return le16toh(x); }
template<> int32_t x801::base::convLER<int32_t>(int32_t x) { return le32toh(x); }
template<> int64_t x801::base::convLER<int64_t>(int64_t x) { return le64toh(x); }
template<> int16_t x801::base::convLEW<int16_t>(int16_t x) { return htole16(x); }
template<> int32_t x801::base::convLEW<int32_t>(int32_t x) { return htole32(x); }
template<> int64_t x801::base::convLEW<int64_t>(int64_t x) { return htole64(x); }

std::stringstream x801::base::fromCharArray(char* array, unsigned int size) {
  std::string s{array, size};
  return std::stringstream(s);
}

// Working on making this code more idiomatic, since it came from a PH3
// archive extractor written in C.
int x801::base::readZipped(std::istream& f, char*& block, uint32_t& amtReadC, uint32_t& amtReadU) {
  unsigned int bsize = 1;
  int ret = Z_OK;
  char* src = (char*) malloc(CHUNK);
  char* dest = (char*) malloc(bsize * CHUNK);
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit(&strm);
  if (ret != Z_OK) goto end;
  do {
    f.read(src, CHUNK);
    strm.avail_in = f.gcount();
    if (f.bad()) {
      ret = Z_ERRNO;
      break;
    }
    if (strm.avail_in == 0)
      break;
    strm.next_in = (unsigned char*) src;
    do {
      if (strm.total_out > CHUNK * (bsize - 1)) {
        bsize <<= 1;
        dest = (char*) realloc(dest, bsize * CHUNK);
      }
      strm.avail_out = CHUNK;
      strm.next_out = (unsigned char*) (dest + strm.total_out);
      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
      case Z_NEED_DICT:
          ret = Z_DATA_ERROR;     /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
          goto end;
          break;
        default:
          ret = Z_OK;
      }
    } while (strm.avail_out == 0);
    //fprintf(stderr, "Unzip: total_out %lu available space %d\n", strm.total_out, CHUNK * bsize);
  } while (strm.avail_out != 0);
  end:
  amtReadC = strm.total_in;
  amtReadU = strm.total_out;
  free(src);
  if (ret == 0) block = dest;
  else {
    free(dest);
    block = nullptr;
  }
  (void) inflateEnd(&strm);
  return ret;
}

// Also based on zlib usage example doc but simpler because we're compressing
// from a buffer, not a file.
int x801::base::writeZipped(std::ostream& f, const char* block, uint32_t len, uint32_t& amtWrittenC) {
  int ret = Z_OK;
  char* dest = (char*) malloc(CHUNK);
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = len;
  strm.next_in = (unsigned char*) block;
  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  if (ret != Z_OK) goto end;
  do {
    strm.avail_out = CHUNK;
    strm.next_out = (unsigned char*) (dest);
    ret = deflate(&strm, Z_FINISH);
    //std::clog << "Zip status " << ret << '\n';
    if (ret == Z_STREAM_ERROR) goto end;
    int have = CHUNK - strm.avail_out;
    f.write(dest, have);
    if (f.bad()) {
      ret = Z_ERRNO;
      goto end;
    }
    switch (ret) {
    case Z_NEED_DICT:
        ret = Z_DATA_ERROR;     /* and fall through */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
        goto end;
        break;
      default:
        ret = Z_OK;
    }
  } while (strm.avail_out == 0);
  assert(strm.avail_in == 0);
  //fprintf(stderr, "Zip: total_out %lu available space %d\n", strm.total_out, CHUNK);
  end:
  amtWrittenC = strm.total_out;
  free(dest);
  (void) deflateEnd(&strm);
  return ret;
}
