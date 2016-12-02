#include "Test.h"

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

using namespace x801::test;

#include <stdint.h>
#include <string.h>
#include <sstream>
#include <utils.h>

#define shouldTest(index) (isDefault || !strcmp(arg, parts[index]))

int main(int argc, char** argv) {
  const char* arg = argc >= 2 ? argv[1] : DEFAULT;
  bool isDefault = argc <= 1 || !strcmp(arg, DEFAULT);
  if (!strcmp(arg, "list")) {
    std::cout << "List of tests:\n";
    for (int i = 0; i < partCount; ++i)
      std::cout << " " << parts[i];
    std::cout << "\n\n";
    return 0;
  } else if (!strcmp(arg, parts[0])) {
    assertThat(5 - 3 == 2, "Arithmetic should work");
    assertEqual(2 + 2, 5, "Arithmetic should NOT work");
    assertEqual("foe", "foe", "Comparing two const char*'s");
    goto cleanup;
  } else if (!strcmp(arg, parts[1])) {
    assertThat(5 - 3 == 2, "Arithmetic should work");
    assertEqual(2 + 2, 4, "Arithmetic should STILL work");
    goto cleanup;
  }
  if (shouldTest(2)) {
    std::stringstream input("GreyroseIsDank");
    uint64_t noun = x801::map::readInt<uint64_t>(input);
    uint16_t verb = x801::map::readInt<uint16_t>(input);
    uint32_t adj = x801::map::readInt<uint32_t>(input);
    // Needs the explicit cast or assertEqual will complain about not finding
    // the version of assertEqualPrivate with the right signature.
    assertEqual(noun, (uint64_t) 0x65736f7279657247LL,
      "Reading uint64_t in LE");
    assertEqual(verb, (uint16_t) 0x7349,
      "Reading uint16_t in LE");
    assertEqual(adj, (uint32_t) 0x6b6e6144L,
      "Reading uint32_t in LE");
  }
  if (shouldTest(3)) {
    std::stringstream output;
    x801::map::writeInt(output, (uint64_t) 0x65736f7279657247LL);
    x801::map::writeInt(output, (uint16_t) 0x7349);
    x801::map::writeInt(output, (uint32_t) 0x6b6e6144L);
    assertEqual(output.str(), "GreyroseIsDank", "Writing integers in LE");
  }
  cleanup:
  summary();
}

#undef shouldTest

const char* x801::test::DEFAULT = "default";
const char* x801::test::parts[] = {
  "testsystem", "testsystem2", // 2
  "utilRead", "utilWrite", // 4
};
const int x801::test::partCount = sizeof(parts) / sizeof(*parts);
