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
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <sstream>
#include <vector>
#include <Area.h>
#include <Database.h>
#include <Layer.h>
#include <TileSec.h>
#include <Version.h>
#include <mapErrors.h>
#include <utils.h>

int main(int argc, char** argv) {
  const char* arg = argc >= 2 ? argv[1] : DEFAULT;
  bool isDefault = argc <= 1 || !strcmp(arg, DEFAULT);
  if (!strcmp(arg, "list")) {
    std::cout << "List of tests:\n";
    for (int i = 0; i < partCount; ++i)
      std::cout << " " << parts[i].name;
    std::cout << "\n\n";
    return 0;
  } else runAll(parts, partCount, arg, isDefault);
  summary();
}

void testSystem() {
  assertThat(5 - 3 == 2, "Arithmetic should work (this should pass)");
  assertEqual(2 + 2, 5, "Arithmetic should NOT work (this should fail)");
  assertEqual("foe", "foe", "Comparing two const char*'s (this should pass)");
  std::vector<int> a;
  std::vector<int> b;
  a.push_back(1);
  b.push_back(2);
  assertEqual(a[0], b[0], "First elements should match (this should fail)");
  assertEqual(a, b, "Vectors should match (this should fail)");
}

void testSystem2() {
  assertThat(5 - 3 == 2, "Arithmetic should work");
  assertEqual(2 + 2, 4, "Arithmetic should STILL work");
}

void testReadInt() {
  std::stringstream input("GreyroseIsDank");
  uint64_t noun = x801::base::readInt<uint64_t>(input);
  uint16_t verb = x801::base::readInt<uint16_t>(input);
  uint32_t adj = x801::base::readInt<uint32_t>(input);
  // Needs the explicit cast or assertEqual will complain about not finding
  // the version of assertEqualPrivate with the right signature.
  assertEqual(noun, (uint64_t) 0x65736f7279657247LL,
    "Reading uint64_t in LE");
  assertEqual(verb, (uint16_t) 0x7349,
    "Reading uint16_t in LE");
  assertEqual(adj, (uint32_t) 0x6b6e6144L,
    "Reading uint32_t in LE");
  std::stringstream foo = x801::base::fromCharArray((char*) ("5\0""5\0"), 4);
  uint32_t awk = x801::base::readInt<uint32_t>(foo);
  assertEqual(awk, (uint32_t) 0x350035, "Just a stress test");
  assertThat(foo.good(), "Just a stress test");
}

void testWriteInt() {
  std::stringstream output;
  x801::base::writeInt(output, (uint64_t) 0x65736f7279657247LL);
  x801::base::writeInt(output, (uint16_t) 0x7349);
  x801::base::writeInt(output, (uint32_t) 0x6b6e6144L);
  assertEqual(output.str(), "GreyroseIsDank", "Writing integers in LE");
}

void testVersionBasic() {
  x801::base::Version mine17(1, 7, 10);
  assertEqual(mine17.vMajor, 1, "Major field");
  assertEqual(mine17.vMinor, 7, "Minor field");
  assertEqual(mine17.vPatch, 10, "Patch field");
  assertEqual(mine17.prerelease, 0xc000, "Prerelease field");
  assertEqual(mine17.getPrereleaseType(), x801::base::RELEASE,
    "Version is release");
  assertEqual(mine17.getPrereleaseNumber(), 0,
    "Release number is 0");
  x801::base::Version greatest(1, 7, 3, x801::base::BETA, 5);
  assertEqual(greatest.prerelease, 0x4005, "Prerelease field (again)");
  assertEqual(mine17, x801::base::Version(1, 7, 10), "Version equality");
}

void testVersionRead() {
  std::stringstream input("Celestia");
  x801::base::Version v(input);
  assertEqual(v.vMajor, 0x6543, "Major field read");
  assertEqual(v.vMinor, 0x656c, "Minor field read");
  assertEqual(v.vPatch, 0x7473, "Revision field read");
  assertEqual(v.getPrereleaseType(), x801::base::BETA, "Correct pretype");
  assertEqual(v.getPrereleaseNumber(), 0x2169, "Correct prenum");
}

#define W "\x01\x00\x00\x80"
#define O "\x00\x00\x00\x00"
#define NORTH_OR_SOUTH W W W W W W W W
#define CENTER W O O O O O O W
#define NORTH_OR_SOUTH_2 W W NORTH_OR_SOUTH W W
#define EMPTY_ROW_2 W O O O O O O O O O O W
#define SQUARE_EDGE_ROW_2 W O O W W W W W W O O W
#define SQUARE_SIDE_ROW_2 W O O W O O O O W O O W

void testLayerIO() {
  // Make an 8x8 map with the northwest corner being (-2, -5).
  std::string s = x801::base::construct(
    "\x08\x00\x08\x00" // Dimensions
    "\xfe\xff\xfb\xff" // Offset of NW corner
    NORTH_OR_SOUTH
    CENTER CENTER CENTER
    CENTER CENTER CENTER
    NORTH_OR_SOUTH
  );
  std::stringstream input(s, std::ios_base::in | std::ios_base::binary);
  x801::map::Layer layer(input);
  assertEqual(layer.getWidth(), 8, "Width should be 8");
  assertEqual(layer.getHeight(), 8, "Height should be 8");
  assertEqual(layer.getXOffset(), -2, "Offset should be (-2, -5)");
  assertEqual(layer.getYOffset(), -5, "Offset should be (-2, -5)");
  x801::map::Block wall(0x80000001);
  x801::map::Block space(0);
  x801::map::Block shouldBeWall = layer.getMapBlockAt(-2, 1);
  assertEqual(shouldBeWall, wall, "Wall on west edge");
  shouldBeWall = layer.getMapBlockAt(5, 0);
  assertEqual(shouldBeWall, wall, "Wall on east edge");
  shouldBeWall = layer.getMapBlockAt(3, 2);
  assertEqual(shouldBeWall, wall, "Wall on south edge");
  shouldBeWall = layer.getMapBlockAt(2, -5);
  assertEqual(shouldBeWall, wall, "Wall on north edge");
  x801::map::Block shouldBeSpace = layer.getMapBlockAt(0, 1);
  assertEqual(shouldBeSpace, space, "Space in center");
  layer.setMapBlockAt(0, -1, wall);
  shouldBeWall = layer.getMapBlockAt(0, -1);
  assertEqual(shouldBeWall, wall, "Newly-set wall");
  layer.setMapBlockAt(0, -1, space); // Revert the change
  std::stringstream output(std::ios_base::out | std::ios_base::binary);
  layer.write(output);
  assertEqual(output.str(), s, "Input and output match");
}

void testTileSecIO() {
  // Make a two-layer map.
  // The first layer is the same as before.
  // The second is bigger and has a square at the center.
  std::string s = x801::base::construct(
    "\x02\x00"
    // Layer 0
    "\x08\x00\x08\x00" // Dimensions
    "\xfe\xff\xfb\xff" // Offset of NW corner
    NORTH_OR_SOUTH
    CENTER CENTER CENTER
    CENTER CENTER CENTER
    NORTH_OR_SOUTH
    // Layer 1
    "\x0c\x00\x0c\x00"
    "\xfa\xff\xf7\xff" // (-6, -9)
    NORTH_OR_SOUTH_2
    EMPTY_ROW_2 EMPTY_ROW_2
    SQUARE_EDGE_ROW_2
    SQUARE_SIDE_ROW_2 SQUARE_SIDE_ROW_2
    SQUARE_SIDE_ROW_2 SQUARE_SIDE_ROW_2
    SQUARE_EDGE_ROW_2
    EMPTY_ROW_2 EMPTY_ROW_2
    NORTH_OR_SOUTH_2
  );
  std::stringstream input(s, std::ios_base::in | std::ios_base::binary);
  x801::map::TileSec ts(input);
  assertEqual(ts[0].getWidth(), 8, "Layer 0 of TileSec probably read fine");
  assertEqual(ts[1].getWidth(), 12, "Layer 1 of TileSec probably read fine");
  std::stringstream output(std::ios_base::out | std::ios_base::binary);
  ts.write(output);
  assertEqual(output.str(), s, "Input and output match");
}

void testAreaIO() {
  // Make a two-layer map.
  // The first layer is the same as before.
  // The second is bigger and has a square at the center.
  std::string s = x801::base::construct(
    "XMap" // magic number
    "\x00\x00\x00\x00\x00\x00\x00\x00" // version
    "\x03\x00\x03\x00" // World 3 Area 3
    "\x01\x00\x00\x00" // This world has one data section.
    // Data Section 0
    "TILE" // id
    "\x52\x03\x00\x00" // this is 850 bytes long
    "\x00\x00\x00\x00"
    "\x02\x00"
    // Layer 0
    "\x08\x00\x08\x00" // Dimensions
    "\xfe\xff\xfb\xff" // Offset of NW corner
    NORTH_OR_SOUTH
    CENTER CENTER CENTER
    CENTER CENTER CENTER
    NORTH_OR_SOUTH
    // Layer 1
    "\x0c\x00\x0c\x00"
    "\xfa\xff\xf7\xff" // (-6, -9)
    NORTH_OR_SOUTH_2
    EMPTY_ROW_2 EMPTY_ROW_2
    SQUARE_EDGE_ROW_2
    SQUARE_SIDE_ROW_2 SQUARE_SIDE_ROW_2
    SQUARE_SIDE_ROW_2 SQUARE_SIDE_ROW_2
    SQUARE_EDGE_ROW_2
    EMPTY_ROW_2 EMPTY_ROW_2
    NORTH_OR_SOUTH_2
  );
  std::stringstream input(s, std::ios_base::in | std::ios_base::binary);
  x801::map::Area area(input, true);
  std::stringstream output(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  area.write(output);
  output.seekg(0);
  // std::string str = output.str();
  // feed(std::cout, str);
  x801::map::Area area2(output);
  assertEqual(area2.getError(), x801::map::MAPERR_OK, "Map should be read without error");
  std::stringstream output2(std::ios_base::out | std::ios_base::binary);
  area2.write(output2);
  assertEqual(output.str(), output2.str(), "Outputs match");
}

#undef W
#undef O
#undef NORTH_OR_SOUTH
#undef CENTER
#undef NORTH_OR_SOUTH_2
#undef EMPTY_ROW_2
#undef SQUARE_EDGE_ROW_2
#undef SQUARE_SIDE_ROW_2

void testDBAuth() {
  system("rm -rf `dirname $0`/saves");
  x801::game::Database db;
  // createAuthTable implicit
  db.createUserDebug("ウルヰ", "GGLuisLifeHaven");
  db.createUserDebug("TestUser", "ILoveToTest");
}

const char* x801::test::DEFAULT = "default";
const Test x801::test::parts[] = {
  {"testSystem", testSystem, false},
  {"testSystem2", testSystem2, false},
  {"readInt", testReadInt, true},
  {"writeInt", testWriteInt, true},
  {"versionBasic", testVersionBasic, true},
  {"versionRead", testVersionRead, true},
  {"layer", testLayerIO, true},
  {"tileSec", testTileSecIO, true},
  {"area", testAreaIO, true},
  {"dbAuth", testDBAuth, true},
};
const int x801::test::partCount = sizeof(parts) / sizeof(*parts);

void x801::test::Test::run(const char* arg, bool isDefault) const {
  try {
    if ((isDefault && runByDefault) || !strcmp(arg, name)) test();
  } catch (std::exception& x) {
    std::cerr << "\033[31;1mTEST FAILED!!! \u2013 \n";
    std::cerr << "Exception thrown: \n  ";
    std::cerr << x.what() << '\n';
  } catch (const char* s) {
    std::cerr << "\033[31;1mTEST FAILED!!! \u2013 \n";
    std::cerr << "Exception thrown: \n  ";
    std::cerr << s << '\n';
  }
}

void x801::test::runAll(
    const Test* tests,
    int count,
    const char* arg,
    bool isDefault) {
  for (int i = 0; i < count; ++i) tests[i].run(arg, isDefault);
}
