#pragma once

#include <stddef.h>

#define GLEW_STATIC
#include <GL/glew.h>

namespace x801 {
  namespace game {
    extern size_t texsize;
    void getTexsize();
  }
}