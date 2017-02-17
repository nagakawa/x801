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
#include <atomic>
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include "CircularQueue.h"

namespace x801 {
  namespace base {
    template<typename T>
    class ConcurrentCircularQueue {
    public:
      ConcurrentCircularQueue(size_t capacity = CIRCULAR_QUEUE_DEFAULT_CAPACITY) :
          capacity(capacity), elements(new T[1 << capacity]),
          start(0), totalElements(0) {}
      ConcurrentCircularQueue(const ConcurrentCircularQueue& that) :
          capacity(that.capacity), elements(new T[1 << that.capacity]),
          start(that.start), totalElements(that.totalElements) {
        boost::unique_lock<boost::shared_mutex> tguard(that.pointerMutex);
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        for (size_t i = 0; i < totalElements; ++i) {
          (*this)[i] = that[i];
        }
      }
      ConcurrentCircularQueue& operator=(const ConcurrentCircularQueue& that) {
        if (capacity != that.capacity) {
          capacity = that.capacity;
          pointerMutex.lock();
          delete[] elements;
          elements = new T[1 << capacity];
          pointerMutex.unlock();
        }
        boost::unique_lock<boost::shared_mutex> tguard(that.pointerMutex);
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        start = that.start;
        totalElements = that.totalElements;
        for (size_t i = 0; i < totalElements; ++i) {
          (*this)[i] = that[i];
        }
      }
      ~ConcurrentCircularQueue() { delete[] elements; }
      // ---------------------------------------------------------------------
      T& operator[](size_t i) {
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        return elements[(start + i) & ((1 << capacity) - 1)];
      }
      const T& operator[](size_t i) const {
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        return elements[(start + i) & ((1 << capacity) - 1)];
      }
      void pushFront(const T& t) {
        makeRoomForFront();
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        elements[start] = t;
      }
      void pushBack(const T& t) {
        makeRoomForBack();
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        elements[(start + totalElements - 1) & ((1 << capacity) - 1)] = t;
      }
      void popFront(size_t n = 1) {
        if (totalElements == 0) return;
        totalElements -= n;
        start += n;
      }
      void popBack(size_t n = 1) {
        if (totalElements == 0) return;
        totalElements -= n;
      }
      template<class... Args>
      void emplaceFront(Args&&... args) {
        makeRoomForFront();
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
        std::allocator_traits<std::allocator<T>>::construct(
            allocator, &elements[start], args...);
      }
      template<class... Args>
      void emplaceBack(Args&&... args) {
        makeRoomForBack();
        boost::unique_lock<boost::shared_mutex> guard(pointerMutex);
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
        pointerMutex.lock();
        for (size_t i = 0; i < totalElements; ++i) {
          newElements[(start + i) & ((1 << capacity) - 1)]
            = elements[(start + i) & ((1 << (capacity - 1)) - 1)];
        }
        delete[] elements;
        elements = newElements;
        pointerMutex.unlock();
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
      std::atomic_size_t capacity = CIRCULAR_QUEUE_DEFAULT_CAPACITY;
      mutable boost::shared_mutex pointerMutex;
      T* elements = nullptr;
      // The maximum number of elements is actually (1 << capacity),
      // as to use bitwise operations rather than modulus.
      std::atomic_size_t start = 0;
      std::atomic_size_t totalElements = 0;
      std::allocator<T> allocator;
    };
  }
}