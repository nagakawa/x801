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

#include <string.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace x801 {
  namespace test {
    struct TestDiag {
      TestDiag() : passedTests(0), totalTests(0) {}
      int passedTests;
      int totalTests;
      static TestDiag current;
    };
    // This is just template metaprogramming to output a value if it can be
    // and just output a default string if it can't.
    // thanks http://stackoverflow.com/a/22759544/3130218
    template<typename S, typename T>
    class IsStreamable {
      template<typename SS, typename TT>
      static auto test(int)
      -> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());
      template<typename, typename>
      static auto test(...) -> std::false_type;
    public:
      static const bool value = decltype(test<S,T>(0))::value;
    };
    // And http://stackoverflow.com/a/30440624/3130218
    template <
      typename T,
      typename std::enable_if<!IsStreamable<std::ostream, T>::value>
        :: type* = nullptr
    >
    void feed(std::ostream& fh, T& x) {
      fh << "(huh?)";
      (void) x;
    }
    template <
      typename T,
      typename std::enable_if<IsStreamable<std::ostream, T>::value>
        :: type* = nullptr
    >
    void feed(std::ostream& fh, T& x) {
      fh << x;
    }
    template<>
    void feed(std::ostream& fh, std::string& x);
    void assertPrivate(
        bool c,
        const char* what,
        const char* file,
        int line,
        const char* func,
        const char* extra = nullptr);
    template<typename T, typename U> void assertEqualPrivate(
        T a, U b,
        const char* what,
        const char* file,
        int line,
        const char* func) {
      std::stringstream ss;
      feed(ss, a);
      ss << " is not equal to ";
      feed(ss, b);
      std::string s = ss.str();
      assertPrivate(a == b, what, file, line, func, s.c_str());
    }
    template<typename T> void assertApproximatePrivate(
        T a, T b, T e,
        const char* what,
        const char* file,
        int line,
        const char* func) {
      std::stringstream ss;
      feed(ss, a);
      ss << " is approximately equal to ";
      feed(ss, b);
      ss << " with error ";
      feed(ss, e);
      std::string s = ss.str();
      assertPrivate(std::abs(a - b) <= e, what, file, line, func, s.c_str());
    }
    void assertEqualPrivate(
        const char* a, const char* b,
        const char* what,
        const char* file,
        int line,
        const char* func);
    template<typename T, typename U> void assertDifferentPrivate(
        T a, U b,
        const char* what,
        const char* file,
        int line,
        const char* func) {
      std::stringstream ss;
      feed(ss, a);
      ss << " is equal to ";
      feed(ss, b);
      std::string s = ss.str();
      assertPrivate(a != b, what, file, line, func, s.c_str());
    }
    void assertDifferentPrivate(
        const char* a, const char* b,
        const char* what,
        const char* file,
        int line,
        const char* func);
    void summary();
  }
}
