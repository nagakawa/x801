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

#include "TestAux.h"

namespace x801 {
  namespace test {
    //extern TestDiag current;
#define asStr(c) #c
#define assertThat(c, what) \
  x801::test::assertPrivate(c, what, __FILE__, __LINE__, __func__, \
    asStr(c) " evaluates to false")
#define assertEqual(a, b, what) \
  x801::test::assertEqualPrivate(a, b, (const char*) what, \
  (const char*) __FILE__, __LINE__, (const char*) __func__)
#define assertDifferent(a, b, what) \
  x801::test::assertDifferentPrivate(a, b, (const char*) what, \
  (const char*) __FILE__, __LINE__, (const char*) __func__)
    extern const char* DEFAULT;
    bool shouldTest(int index);
    struct Test {
      const char* name;
      void (*test)();
      bool runByDefault;
      void run(const char* arg, bool isDefault) const;
    };
    extern const Test parts[];
    extern const int partCount;
    void runAll(const Test* tests, int count, const char* arg, bool isDefault);
  }
}
