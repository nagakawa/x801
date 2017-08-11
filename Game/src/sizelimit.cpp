#include "sizelimit.h"

#include <algorithm>

namespace x801 {
  namespace game {
    constexpr size_t MAX_TEXSIZE = 4096;
    size_t texsize = -1U;
    void getTexsize() {
      if (texsize != -1U) return;
      GLint size;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
      texsize = std::min(MAX_TEXSIZE, (size_t) size);
    }
  }
}