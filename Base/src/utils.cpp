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
#include <string.h>
#include <sstream>
#include <string>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

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

static_assert(std::numeric_limits<float>::is_iec559, "Float is not IEEE 754!");

float x801::base::readFloat(std::istream& fh) {
  float val;
  fh.read(reinterpret_cast<char*> (&val), sizeof(float));
  return val;
}

void x801::base::writeFloat(std::ostream& fh, float x) {
  fh.write(reinterpret_cast<char*> (&x), sizeof(float));
}

glm::quat&& x801::base::readQuaternion(std::istream& fh) {
  glm::quat q;
  q.x = readFloat(fh);
  q.y = readFloat(fh);
  q.z = readFloat(fh);
  q.w = readFloat(fh);
  return std::move(q);
}

void x801::base::writeQuaternion(std::ostream& fh, const glm::quat& q) {
  writeFloat(fh, q.x);
  writeFloat(fh, q.y);
  writeFloat(fh, q.z);
  writeFloat(fh, q.w);
}

glm::vec3&& readVec3(std::istream& fh) {
  glm::vec3 v;
  v.x = readFloat(fh);
  v.y = readFloat(fh);
  v.z = readFloat(fh);
  return std::move(v);
}

void writeVec3(std::ostream& fh, const glm::vec3& v) {
  writeFloat(fh, v.x);
  writeFloat(fh, v.y);
  writeFloat(fh, v.z);
}

glm::vec2&& readVec2(std::istream& fh) {
  glm::vec2 v;
  v.x = readFloat(fh);
  v.y = readFloat(fh);
  return std::move(v);
}

void writeVec2(std::ostream& fh, const glm::vec2& v) {
  writeFloat(fh, v.x);
  writeFloat(fh, v.y);
}

std::stringstream x801::base::fromCharArray(char* array, unsigned int size) {
  std::string s{array, size};
  return std::stringstream(s);
}

// Working on making this code more idiomatic, since it came from a PH3
// archive extractor written in C.
int x801::base::readZipped(
    std::istream& f,
    char*& block,
    uint32_t& amtReadC,
    uint32_t& amtReadU
) {
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
int x801::base::writeZipped(
    std::ostream& f,
    const char* block,
    uint32_t len,
    uint32_t& amtWrittenC
) {
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

void x801::base::writeRandomBytes(uint8_t* buffer, int length) {
  boost::random::random_device random;
  boost::random::uniform_int_distribution<> dist(0, 255);
  for (int i = 0; i < length; ++i)
    buffer[i] = static_cast<uint8_t>(dist(random));
}

namespace x801 {
  namespace base {
    template<>
    size_t getLength(const char* s) {
      return strlen(s);
    }

    template<>
    size_t getLength(std::string s) {
      return s.length();
    }

    template<>
    const char* getPointer(const char* s) {
      return s;
    }

    template<>
    const char* getPointer(std::string s) {
      return s.c_str();
    }
  }
}

std::string x801::base::slurp(std::ifstream& fh) {
  size_t b = fh.tellg();
  fh.seekg(0, std::ios_base::end);
  size_t e = fh.tellg();
  fh.seekg(b);
  std::string res(e - b, '\0');
  fh.read(&res[0], e - b);
  return res;
}

// Seriously, what the fuck.
#ifdef __WIN32
#include <windows.h>
std::string x801::base::getPathOfCurrentExecutable() {
  char buf[1024];
  uint32_t len = GetModuleFileName(nullptr, buf, 1024);
  if (len == 0) {
    std::cerr << "could not get exe path.\n"
        << "error code: " << GetLastError()
        << "\nalso use a better os\n";
    return "";
  }
  buf[len] = '\0';
  return std::string(buf);
}
#elif defined(__APPLE__) && defined(__MACH__) // OS X
#include <mach-o/dyld.h>
std::string x801::base::getPathOfCurrentExecutable() {
  char buf[1024];
  size_t size = 1024;
  int stat = _NSGetExecutablePath(buf, &size);
  if (stat != 0) {
    std::cerr << "could not get exe path\n";
    return "";
  }
  buf[size] = '\0';
  return std::string(buf);
}
#elif defined(__gnu_linux__)
#include <unistd.h>
std::string x801::base::getPathOfCurrentExecutable() {
  char buf[1024];
  ssize_t len = readlink("/proc/self/exe", buf, 1023);
  if (len == -1) {
    perror("readlink");
    return 0;
  }
  buf[len] = '\0';
  return std::string(buf);
}
#else
#error "your OS isn't supported; contact Uruwi for help"
#endif

bool x801::base::canBeConvertedToPositiveInt(const char* s, int* out) {
  int numberOfDigits = strspn(s, "0123456789");
  // there are more non-digit characters
  if (s[numberOfDigits] != '\0') return false;
  if (out != nullptr) {
    *out = atoi(s);
  }
  return true;
}
