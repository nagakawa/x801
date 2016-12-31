#pragma once

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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error Only C++11 or later supported.
#endif

#include <stdint.h>
#include <iostream>
#include <Version.h>
#include "QualifiedAreaID.h"
#include "TileSec.h"

namespace x801 {
  namespace map {
    class Area {
    public:
      Area(std::istream& fh, bool dontCare = false);
      void write(std::ostream& fh) const;
      ~Area();
      Area(const Area& that) = delete;
      void operator=(const Area& that) = delete;
      int getError() const { return error; }
      QualifiedAreaID getQualifiedAreaID() { return id; }
      TileSec& getTileSec() { return *ts; }
    private:
      x801::base::Version version;
      QualifiedAreaID id;
      TileSec* ts = nullptr;
      int readSection(std::istream& fh, bool dontCare);
      void writeTileSection(std::ostream& fh, int& ds) const;
      void writeSection(std::ostream& fh, uint32_t sectionID, const char* data, uint32_t len, int& ds) const;
      int error;
      int index;
    };
  }
}
