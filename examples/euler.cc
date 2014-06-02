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

#include <cstdio>

#include <string>

#include "fn/fn.h"

using fn::_;
using fn::range;

int e1() {
  return  _(range(1, 1000))
              .filter([](int i) { return i % 3 == 0 || i % 5 == 0; })
              .sum();
}

int e4() {
  return _(range(999, 99, -1))
             .flat_map([](int i) {
                return _(range(999, i, -1))
                    .map([i](int j) { return i * j; });
              })
             .filter([](int i) {
                auto s = std::to_string(i);
                return std::equal(s.begin(), s.end(), s.rbegin());
              })
             .max();
}

int main() {
  printf("Solution to Euler #1 is %d.\n", e1());
  printf("Solution to Euler #4 is %d.\n", e4());
}

