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

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <sstream>
#include <vector>
#include <boost/filesystem.hpp>
#include <Area.h>
#include <Chunk.h>
#include <CircularQueue.h>
#include <Database.h>
#include <Location.h>
#include <Model.h>
#include <TileSec.h>
#include <Version.h>
#include <mapErrors.h>
#include <utils.h>

int main(int argc, char** argv) {
  std::cout << "The path of the test exe is " <<
      x801::base::getPathOfCurrentExecutable() << '\n';
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

#include <test_chunk.h>

void testChunkIO() {
  std::string s = x801::base::construct(
    "\x02\x00\x03\x00\xfe\xff" // Location
    "\x00\x00" // Not empty
    A_CHUNK
  );
  std::stringstream input(s, std::ios_base::in | std::ios_base::binary);
  x801::map::Chunk chunk(input);
  assertEqual(chunk.getX(), 2, "X should be 2");
  assertEqual(chunk.getY(), 3, "Y should be 3");
  assertEqual(chunk.getZ(), -2, "Z should be -2");
  assertEqual(chunk.isEmpty(), false, "Chunk should not be empty");
  x801::map::Block wall(0x80000001);
  x801::map::Block space(0);
  x801::map::Block shouldBeWall = chunk.getMapBlockAt(0, 5, 3);
  assertEqual(shouldBeWall, wall, "Wall on west edge");
  shouldBeWall = chunk.getMapBlockAt(15, 7, 2);
  assertEqual(shouldBeWall, wall, "Wall on east edge");
  shouldBeWall = chunk.getMapBlockAt(6, 0, 1);
  assertEqual(shouldBeWall, wall, "Wall on north edge");
  shouldBeWall = chunk.getMapBlockAt(3, 15, 2);
  assertEqual(shouldBeWall, wall, "Wall on south edge");
  shouldBeWall = chunk.getMapBlockAt(7, 9, 0);
  assertEqual(shouldBeWall, wall, "Wall on floor (wtf?)");
  x801::map::Block shouldBeSpace = chunk.getMapBlockAt(5, 5, 7);
  assertEqual(shouldBeSpace, space, "Space in center");
  chunk.setMapBlockAt(5, 5, 7, wall);
  shouldBeWall = chunk.getMapBlockAt(5, 5, 7);
  assertEqual(shouldBeWall, wall, "Newly-set wall");
  chunk.setMapBlockAt(5, 5, 7, space); // Revert the change
  std::stringstream output(std::ios_base::out | std::ios_base::binary);
  chunk.write(output);
  assertEqual(output.str(), s, "Input and output match");
}

void testTileSecIO() {
  // Make a two-chunk map.
  // They are adjacent and identical.
  std::string s = x801::base::construct(
    "\x02\x00"
    // Chunk 1
    "\x02\x00\x03\x00\xfe\xff" // Location
    "\x00\x00" // Not empty
    A_CHUNK
    // Chunk 0
    "\x03\x00\x03\x00\xfe\xff" // Location
    "\x00\x00" // Not empty
    A_CHUNK
  );
  std::stringstream input(s, std::ios_base::in | std::ios_base::binary);
  x801::map::TileSec ts(input);
  x801::map::Block wall(0x80000001);
  x801::map::Block shouldBeWall =
    ts.getBlock(x801::map::BlockXYZ(35, 63, -30));
  assertEqual(shouldBeWall, wall, "Wall on south edge of chunk 1");
  std::stringstream output(std::ios_base::out | std::ios_base::binary);
  ts.write(output);
  assertEqual(output.str(), s, "Input and output match");
}

void testAreaIO() {
  // Make a chunk map.
  std::string s = x801::base::construct(
    "XMap" // magic number
    "\x00\x00\x00\x00\x00\x00\x00\x00" // version
    "\x03\x00\x03\x00" // World 3 Area 3
    "\x01\x00\x00\x00" // This world has one data section.
    // Data Section 0
    "TIL3" // id
    "\x12\x80\x00\x00" // this is 32786 bytes long
    "\x00\x00\x00\x00"
    "\x02\x00"
    // Chunk 1
    "\x02\x00\x03\x00\xfe\xff" // Location
    "\x00\x00" // Not empty
    A_CHUNK
    // Chunk 0
    "\x03\x00\x03\x00\xfe\xff" // Location
    "\x00\x00" // Not empty
    A_CHUNK
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


void testDBAuth() {
  boost::filesystem::path p = x801::base::getPathOfCurrentExecutable();
  // Fun fact:
  // The original code said
  // boost::filesystem::remove_all(p);
  // Which, instead of deleting the directory with the saves, destroyed the
  // executable itself, causing hashes to not be updated when we decided to
  // use SHA-256 over SHA-1.
  boost::filesystem::remove_all(p.parent_path() / "saves");
  x801::game::Database db;
  // createAuthTable implicit
  db.createUserDebug("Uruwi", "GGLuisLifeHaven");
  db.createUserDebug("TestUser", "ILoveToTest");
  x801::game::StoredCredentials sc;
  bool success = db.getUserByID(1, sc);
  assertThat(success, "Get user with id = 1");
  assertEqual(sc.getUsernameS(), "Uruwi", "User with id = 1 is Uruwi");
  assertThat(!db.getUserByID(9, sc), "No user with id = 9");
  success = db.getUserByName("TestUser", sc);
  assertThat(success, "Get user with username = TestUser");
  assertEqual(sc.getUserID(), 2U, "User with username = TestUser is user #2");
  assertThat(!db.getUserByName("MM101", sc), "No user with username = MM101");
  int id = db.getUserIDByName("Uruwi");
  assertEqual(id, 1, "ID of Uruwi is 1");
  id = db.getUserIDByName("Dworgyn");
  assertEqual(id, 0, "User Dworgyn does not exist, so 0 is returned");
  x801::game::Location locationOfUruwi = {
    { 0, 0 }, // Presumably messing around in the x801 counterpart of the Commons?
    3.7f, 9.2f, 0.0f, -0.14f
  };
  db.savePlayerLocation(1, locationOfUruwi);
  x801::game::Location shouldBeSame;
  db.loadPlayerLocation(1, shouldBeSame);
  assertThat(
    fabs(locationOfUruwi.x - shouldBeSame.x) < 1e-5f,
    "Uruwi is at about the same x-coordinate"
  );
  x801::game::Credentials attempt1("TestUser", "ILoveToTest");
  assertThat(attempt1.matches(sc), "Correct password input");
  x801::game::Credentials attempt2("TestUser", "WrongPassword");
  assertThat(!attempt2.matches(sc), "Wrong password input");
}

// Toy class to test CircularQueue
struct Cup {
  std::string drink;
  int volumeInML;
  Cup() : drink("water"), volumeInML(500)  {}
  Cup(const std::string& drink, int volumeInML) :
    drink(std::move(drink)), volumeInML(volumeInML) {}
  bool operator==(const Cup& other) {
    return drink == other.drink && volumeInML == other.volumeInML;
  }
};

std::ostream& operator<<(std::ostream& stream, const Cup& cup) {
  stream << "Cup(" << cup.drink << ", " << cup.volumeInML << ")";
  return stream;
}

void testCircularQueue() {
  x801::base::CircularQueue<Cup> cups;
  cups.pushBack(Cup("Sprite", 450));
  cups.emplaceBack("Fanta", 400);
  assertEqual(cups[0], Cup("Sprite", 450), "CircularQueue::pushBack works");
  assertEqual(cups[1], Cup("Fanta", 400), "CircularQueue::emplaceBack works");
  cups.emplaceFront("orange juice", 250);
  assertEqual(cups[0], Cup("orange juice", 250), "CircularQueue::emplaceBack works");
  assertEqual(cups[1], Cup("Sprite", 450), "CircularQueue::emplaceFront shifts elements");
  for (int i = 0; i < 100; ++i) {
    cups.emplaceFront("Mountain Dew", 1000 + i);
    cups.emplaceBack("milk", 2000 + i);
  }
  assertEqual(cups.size(), 203U, "Container has correct number of elements");
  assertEqual(cups[37].volumeInML, 1062,
    "Arbitrary element selected near front");
  assertEqual(cups[cups.size() - 1 - 69].volumeInML, 2030,
    "Arbitrary element selected near back");
  cups.popFront();
  cups.popBack();
  assertEqual(cups.size(), 201U, "Container has correct number of elements");
  assertEqual(cups[36].volumeInML, 1062,
    "Same element has index that is one less than before front was popped");
}

void testModelFunctionIO() {
  using namespace std::literals::string_literals;
  // Taken from block.cmf
  std::string bdata =
    "\x01\x00\x3F\x06\x08\x00\x0C\x00\xC0\xC0\x40\x40\xC0\x40\x40\x40"
    "\x40\xC0\x40\x40\x40\xC0\xC0\xC0\xC0\xC0\xC0\x40\xC0\x40\x40\xC0"
    "\x00\x00\x01\x00\x02\x00\x00\x00\x80\x00\x80\x80\x00\x01\x02\x00"
    "\x03\x00\x00\x00\x80\x80\x00\x80\x00\x00\x00\x01\x04\x00\x05\x00"
    "\x06\x00\x00\x80\x80\x00\x80\x80\x01\x02\x06\x00\x07\x00\x04\x00"
    "\x80\x80\x00\x80\x00\x80\x01\x02\x02\x00\x03\x00\x06\x00\x00\x00"
    "\x80\x00\x80\x80\x02\x04\x06\x00\x07\x00\x02\x00\x80\x80\x00\x80"
    "\x00\x00\x02\x04\x00\x00\x01\x00\x04\x00\x00\x00\x80\x00\x80\x80"
    "\x03\x08\x04\x00\x05\x00\x00\x00\x80\x80\x00\x80\x00\x00\x03\x08"
    "\x01\x00\x02\x00\x07\x00\x00\x00\x80\x00\x80\x80\x04\x10\x07\x00"
    "\x04\x00\x01\x00\x80\x80\x00\x80\x00\x00\x04\x10\x03\x00\x00\x00"
    "\x05\x00\x00\x00\x80\x00\x80\x80\x05\x20\x05\x00\x06\x00\x03\x00"
    "\x80\x80\x00\x80\x00\x00\x05\x20"
    ""s;
  std::stringstream input(bdata);
  x801::map::ModelFunction block(input);
  assertEqual(block.hitboxType, x801::map::HitboxType::HITBOX_FULL,
    "Read that this is a full block");
  assertEqual(block.textureCount, 6, "Uses 6 textures");
  // First vertex is nnp.
  assertEqual(block.vertices[0].x, -64, "Vertex 0 has X of -64 (-0.5)");
  assertEqual(block.vertices[0].y, -64, "Vertex 0 has X of -64 (-0.5)");
  assertEqual(block.vertices[0].z, 64, "Vertex 0 has X of +64 (+0.5)");
  // First face is 0 - 1 - 2
  assertEqual(block.faces[0].vertices[0].index, 0,
    "First vertex of face 0 is vertex 0");
  assertEqual(block.faces[0].vertices[0].u, 0,
    "First vertex of face 0 has u = 0");
  assertEqual(block.faces[0].vertices[0].v, 0,
    "First vertex of face 0 has v = 0");
  std::stringstream output;
  block.write(output);
  std::string outData = output.str();
  assertEqual(outData, bdata, "Input and output are equal");
}

void testModelFunctionIndexIO() {
  using namespace std::literals::string_literals;
  std::string adata =
    "\x58\x4D\x44\x46\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00"
    "\x01\x00\x3F\x06\x08\x00\x0C\x00\xC0\xC0\x40\x40\xC0\x40\x40\x40"
    "\x40\xC0\x40\x40\x40\xC0\xC0\xC0\xC0\xC0\xC0\x40\xC0\x40\x40\xC0"
    "\x00\x00\x01\x00\x02\x00\x00\x00\x80\x00\x80\x80\x00\x01\x02\x00"
    "\x03\x00\x00\x00\x80\x80\x00\x80\x00\x00\x00\x01\x04\x00\x05\x00"
    "\x06\x00\x00\x80\x80\x00\x80\x80\x01\x02\x06\x00\x07\x00\x04\x00"
    "\x80\x80\x00\x80\x00\x80\x01\x02\x02\x00\x03\x00\x06\x00\x00\x00"
    "\x80\x00\x80\x80\x02\x04\x06\x00\x07\x00\x02\x00\x80\x80\x00\x80"
    "\x00\x00\x02\x04\x00\x00\x01\x00\x04\x00\x00\x00\x80\x00\x80\x80"
    "\x03\x08\x04\x00\x05\x00\x00\x00\x80\x80\x00\x80\x00\x00\x03\x08"
    "\x01\x00\x02\x00\x07\x00\x00\x00\x80\x00\x80\x80\x04\x10\x07\x00"
    "\x04\x00\x01\x00\x80\x80\x00\x80\x00\x00\x04\x10\x03\x00\x00\x00"
    "\x05\x00\x00\x00\x80\x00\x80\x80\x05\x20\x05\x00\x06\x00\x03\x00"
    "\x80\x80\x00\x80\x00\x00\x05\x20"
    ""s;
  std::stringstream input(adata);
  x801::map::ModelFunctionIndex all(input);
  x801::map::ModelFunction& block = all.models[0];
  assertEqual(block.hitboxType, x801::map::HitboxType::HITBOX_FULL,
    "Read that the first block is a full block");
  assertEqual(block.textureCount, 6, "Uses 6 textures");
  // First vertex is nnp.
  assertEqual(block.vertices[0].x, -64, "Vertex 0 has X of -64 (-0.5)");
  assertEqual(block.vertices[0].y, -64, "Vertex 0 has X of -64 (-0.5)");
  assertEqual(block.vertices[0].z, 64, "Vertex 0 has X of +64 (+0.5)");
  // First face is 0 - 1 - 2
  assertEqual(block.faces[0].vertices[0].index, 0,
    "First vertex of face 0 is vertex 0");
  assertEqual(block.faces[0].vertices[0].u, 0,
    "First vertex of face 0 has u = 0");
  assertEqual(block.faces[0].vertices[0].v, 0,
    "First vertex of face 0 has v = 0");
  std::stringstream output;
  all.write(output);
  std::string outData = output.str();
  assertEqual(outData, adata, "Input and output are equal");
}

const char* x801::test::DEFAULT = "default";
const Test x801::test::parts[] = {
  {"testSystem", testSystem, false},
  {"testSystem2", testSystem2, false},
  {"readInt", testReadInt, true},
  {"writeInt", testWriteInt, true},
  {"versionBasic", testVersionBasic, true},
  {"versionRead", testVersionRead, true},
  {"chunk", testChunkIO, true},
  {"tileSec", testTileSecIO, true},
  {"area", testAreaIO, true},
  {"dbAuth", testDBAuth, true},
  {"circularQueue", testCircularQueue, true},
  {"modelFunctionIO", testModelFunctionIO, true},
  {"modelFunctionIndexIO", testModelFunctionIndexIO, true},
};
const int x801::test::partCount = sizeof(parts) / sizeof(*parts);

void x801::test::Test::run(const char* arg, bool isDefault) const {
  try {
    if ((isDefault && runByDefault) || !strcmp(arg, name)) {
      std::cout << "\033[0;36mRunning the test \033[1m" << name << '\n';
      test();
    }
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
