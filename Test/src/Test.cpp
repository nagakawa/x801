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

#include <string.h>
#include <sstream>
#include <utils.h>

int main(int argc, char** argv) {
  const char* arg = argc >= 2 ? argv[1] : "default";
  bool isDefault = argc <= 1 || !strcmp(arg, "default");
  if (!strcmp(arg, "testsystem")) {
    assertThat(5 - 3 == 2, "Arithmetic should work");
    assertEqual(2 + 2, 5, "Arithmetic should NOT work");
  } else if (!strcmp(arg, "testsystem2")) {
    assertThat(5 - 3 == 2, "Arithmetic should work");
    assertEqual(2 + 2, 4, "Arithmetic should STILL work");
  } else {
    //stringstream()
  }
  (void) isDefault;
  summary();
}
