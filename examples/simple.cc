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
#include <vector>

#include "fn/fn.h"

using fn::_;

void max() {
  std::vector<int> v{2, 1, 3, 5, -4};

  // Use reduce to find max.
  auto max = _(v).reduce([](int m, int i) { return std::max(m, i); });
  printf("Max is %d\n", max);

  // Use max.
  max = _(v).max();
  printf("Max is %d\n", max);

  // And prevent a copy.
  max = _(&v).max();
  printf("Max is %d\n", max);
}

void evens() {
  std::vector<int> v{2, 1, 3, 5, -4};

  // Use filter and evaluate as a vector.
  std::vector<int> evens = _(&v).filter([](int i) { return i % 2 == 0; })
                                .as_vector();
  _(&evens) >> [](int i) {
    printf("%d is even.\n", i);
  };

  // Use filter and append them to a vector.
  evens.clear();
  _(&v).filter([](int i) { return i % 2 == 0; }) >> &evens;
  _(&evens) >> [](int i) {
    printf("%d is even.\n", i);
  };

  // Call for_each.
  _(&v).filter([](int i) { return i % 2 == 0; }).for_each([](int i) {
    printf("%d is even.\n", i);
  });

  // Or use the syntax sugar.
  _(&v).filter([](int i) { return i % 2 == 0; }) >> [](int i) {
    printf("%d is even.\n", i);
  };

  // And another syntax sugar.
  _(&v) % [](int i) { return i % 2 == 0; } >> [](int i) {
    printf("%d is even.\n", i);
  };

  // Or simply iterate over the view.
  for (auto i : _(&v) % [](int i) { return i % 2 == 0; }) {
    printf("%d is even.\n", i);
  }
}

int main() {
  max();
  evens();
  return 0;
}

