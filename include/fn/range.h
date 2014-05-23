// Copyright 2014, The Project fn Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.

#ifndef FUNC_RANGE_H_
#define FUNC_RANGE_H_

#include <iterator>

namespace fn {

// A simple iteratable range class.
template <typename T = int>
class Range {
 public:
  class Iterator : public std::iterator<std::forward_iterator_tag, T> {
   public:
    Iterator(const Range* range, T val);

    const T& operator*() const;
    const T& operator->() const;

    Iterator& operator++();
    Iterator operator++(int);

    bool operator==(const Iterator& that) const;
    bool operator!=(const Iterator& that) const;

   private:
    const Range* range_;
    T val_;
  };

  // For compability with stl. Never used internally.
  using const_iterator = Iterator;
  using iterator = Iterator;

  Range(T from, T to, int step = 1);

  size_t size() const;
  bool empty() const;

  Range::Iterator begin() const;
  Range::Iterator end() const;

 private:
  T from_;
  T to_;
  int step_;
};

template <typename T = int>
Range<T> range(T from, T to, int step = 1) {
  return Range<T>(from, to, step);
}

}  // namespace fn

#include "fn/range-inl.h"

#endif  // FUNC_RANGE_H_

