#include "Atlas.h"

#include <utils.h>

namespace x801 {
  namespace map {
    using namespace x801::base;
    Atlas::Elem::Elem(std::istream& fh) {
      pageno = readInt<uint16_t>(fh);
      x1 = readInt<uint16_t>(fh);
      y1 = readInt<uint16_t>(fh);
      x2 = readInt<uint16_t>(fh);
      y2 = readInt<uint16_t>(fh);
    }
    void Atlas::Elem::write(std::ostream& fh) const {
      writeInt<uint16_t>(fh, pageno);
      writeInt<uint16_t>(fh, x1);
      writeInt<uint16_t>(fh, y1);
      writeInt<uint16_t>(fh, x2);
      writeInt<uint16_t>(fh, y2);
    }
    Atlas::Atlas(std::istream& fh) {
      while (!fh.eof()) {
        std::string name = readString<uint16_t>(fh);
        elems.emplace(name, fh);
      }
    }
    void Atlas::write(std::ostream& fh) const {
      for (const auto& p : elems) {
        writeString<uint16_t>(fh, p.first);
        p.second.write(fh);
      }
    }
  }
}