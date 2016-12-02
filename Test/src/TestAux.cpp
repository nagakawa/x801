#include "TestAux.h"

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

#include <iostream>

TestDiag x801::test::TestDiag::current;

void x801::test::assertPrivate(
    bool c,
    const char* what,
    const char* file,
    int line,
    const char* func,
    const char* extra) {
  ++TestDiag::current.totalTests;
  if (c) {
    std::cout << "\033[32mTest passed: " << what << '\n';
    ++TestDiag::current.passedTests;
  } else {
    // \u2013 is the en-dash
    std::cout << "\033[31;1mTEST FAILED!!! \u2013 \n";
    std::cout << "  \033[0m" << what << '\n';
    if (extra != nullptr) {
      std::cout << "  \033[33m" << extra << '\n';
    }
    std::cout << "  \033[0mat \033[35m" << file << ":\033[36m" << line;
    std::cout << " \033[34m(" << func << ")\n\n";
  }
}

void x801::test::summary() {
  std::cout << "\033[0mPassed " << TestDiag::current.passedTests <<
    " of " << TestDiag::current.totalTests << " tests.\n";
  if (TestDiag::current.passedTests == TestDiag::current.totalTests) {
    std::cout << "\033[33;1mCongratulations, you passed all tests!\033[0m\n";
  }
}

void x801::test::assertEqualPrivate(
    const char* a, const char* b,
    const char* what,
    const char* file,
    int line,
    const char* func) {
  std::stringstream ss;
  ss << a << " is not equal to " << b;
  std::string s = ss.str();
  assertPrivate(!strcmp(a, b), what, file, line, func, s.c_str());
}

void x801::test::assertDifferentPrivate(
    const char* a, const char* b,
    const char* what,
    const char* file,
    int line,
    const char* func) {
  std::stringstream ss;
  ss << a << " is equal to " << b;
  std::string s = ss.str();
  assertPrivate(strcmp(a, b), what, file, line, func, s.c_str());
}
