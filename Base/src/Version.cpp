#include "Version.h"

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

using namespace x801::base;

bool x801::base::Version::operator==(Version& other) {
  return
    vMajor == other.vMajor &&
    vMinor == other.vMinor &&
    vPatch == other.vPatch &&
    prerelease == other.prerelease;
}

bool x801::base::Version::operator<(Version& other) {
  if (vMajor < other.vMajor) return true;
  if (vMajor > other.vMajor) return false;
  if (vMinor < other.vMinor) return true;
  if (vMinor > other.vMinor) return false;
  if (vPatch < other.vPatch) return true;
  if (vPatch > other.vPatch) return false;
  return prerelease < other.prerelease;
}

bool x801::base::Version::canSucceed(Version& other) {
  if (*this == other) return true;
  if (vMajor == 0) return false;
  if (*this < other) return false;
  if (vMajor > other.vMajor) return false;
  // alpha, beta, RCs are incompatible with each other (if different)
  return getPrereleaseType() == 3;
}

void x801::base::Version::write(std::ostream& fh) const {
  writeInt<uint16_t>(fh, vMajor);
  writeInt<uint16_t>(fh, vMinor);
  writeInt<uint16_t>(fh, vPatch);
  writeInt<uint16_t>(fh, prerelease);
}

static const char* typeNames[] = {
  "a", "b", "rc"
};

std::string x801::base::Version::toString() const {
  std::string s = std::to_string(vMajor);
  s += ".";
  s += std::to_string(vMinor);
  s += ".";
  s += std::to_string(vPatch);
  int type = getPrereleaseType();
  int number = getPrereleaseNumber();
  if (type != RELEASE) {
    s += typeNames[type];
    s += std::to_string(number);
  } else if (number != 0) {
    s += "_";
    s += std::to_string(number);
  }
  return s;
}

const Version x801::base::engineVersion(0, 0, 3, ALPHA, 0);
