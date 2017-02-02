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

#include <stddef.h>
#include <stdint.h>
#include <memory>

namespace x801 {
  namespace base {
    const size_t CIRCULAR_QUEUE_DEFAULT_CAPACITY = 5;
    template<typename T>
    class CircularQueue {
    public:
      CircularQueue(size_t capacity = CIRCULAR_QUEUE_DEFAULT_CAPACITY) :
          capacity(capacity), elements(new T[1 << capacity]),
          start(0), totalElements(0) {}
      CircularQueue(const CircularQueue& that) :
          capacity(that.capacity), elements(new T[1 << that.capacity]),
          start(that.start), totalElements(that.totalElements) {
        for (size_t i = 0; i < totalElements; ++i) {
          (*this)[i] = that[i];
        }
      }
      CircularQueue& operator=(const CircularQueue& that) {
        if (capacity != that.capacity) {
          capacity = that.capacity;
          delete[] elements;
          elements = new T[1 << capacity];
        }
        start = that.start;
        totalElements = that.totalElements;
        for (size_t i = 0; i < totalElements; ++i) {
          (*this)[i] = that[i];
        }
      }
      ~CircularQueue() { delete[] elements; }
      // ---------------------------------------------------------------------
      T& operator[](size_t i) {
        return elements[(start + i) & ((1 << capacity) - 1)];
      }
      const T& operator[](size_t i) const {
        return elements[(start + i) & ((1 << capacity) - 1)];
      }
      void pushFront(const T& t) {
        makeRoomForFront();
        elements[start] = t;
      }
      void pushBack(const T& t) {
        makeRoomForBack();
        elements[(start + totalElements - 1) & ((1 << capacity) - 1)] = t;
      }
      void popFront() {
        if (totalElements == 0) return;
        --totalElements;
        ++start;
      }
      void popBack() {
        if (totalElements == 0) return;
        --totalElements;
      }
      template<class... Args>
      void emplaceFront(Args&&... args) {
        makeRoomForFront();
        std::allocator_traits<std::allocator<T>>::construct(
            allocator, &elements[start], args...);
      }
      template<class... Args>
      void emplaceBack(Args&&... args) {
        makeRoomForBack();
        std::allocator_traits<std::allocator<T>>::construct(
          allocator,
          &elements[(start + totalElements - 1) & ((1 << capacity) - 1)],
          args...
        );
      }
      size_t size() { return totalElements; }
    private:
      void grow() {
        ++capacity;
        T* newElements = new T[1 << capacity];
        for (size_t i = 0; i < totalElements; ++i) {
          newElements[(start + i) & ((1 << capacity) - 1)]
            = elements[(start + i) & ((1 << (capacity - 1)) - 1)];
        }
        delete[] elements;
        elements = newElements;
      }
      void makeRoomForFront() {
        ++totalElements;
        if (totalElements > (size_t) (1 << capacity)) grow();
        if (start == 0) start = (1 << capacity) - 1;
        else --start;
      }
      void makeRoomForBack() {
        ++totalElements;
        if (totalElements > (size_t) (1 << capacity)) grow();
      }
      size_t capacity = CIRCULAR_QUEUE_DEFAULT_CAPACITY;
      T* elements = nullptr;
      // The maximum number of elements is actually (1 << capacity),
      // as to use bitwise operations rather than modulus.
      ssize_t start = 0;
      size_t totalElements = 0;
      std::allocator<T> allocator;
    };
  }
}