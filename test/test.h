// Copyright (c) 2014, The Project fn Authors. All rights reserved.
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

#ifndef FUNC_TEST_TEST_H_
#define FUNC_TEST_TEST_H_

#include <iostream>
#include <string>

namespace fn {
namespace test {

// To avoid any dependency, we implement a light-weight gtest-like library.
#define TEST_NAME(g, n) g##_##n
#define TEST_VAR(g, n) g##_##n##__

#define TEST(g, n)                                                \
  void TEST_NAME(g, n)();                                         \
  int TEST_VAR(g, n) = fn::test::add_test([]() { RUN(g, n); }); \
  void TEST_NAME(g, n)()

#define EXPECT_EQ(expected, actual, msg)                               \
  if ((expected) != (actual)) {                                        \
    fn::test::error = true;                                          \
    std::cerr << std::endl << "Error (" << __FILE__ << ":" << __LINE__ \
              << "): " << (msg) << std::endl;                          \
    std::cerr << "\tExpected: " << (expected) << std::endl;            \
    std::cerr << "\tActual: " << (actual) << std::endl;                \
  }

#define EXPECT_TRUE(pred, msg) EXPECT_EQ(true, pred, msg)
#define EXPECT_FALSE(pred, msg) EXPECT_EQ(false, pred, msg)

bool error = false;

enum class Color {
  RESET = 0,
  RED = 31,
  GREEN = 32,
  YELLOW = 33,
  BLUE = 34,
  MAGENTA = 35,
  CYAN = 36,
};

inline std::string color(Color c, bool bold = false) {
  return std::string("\033[") + (bold ? "1;" : "") + std::to_string(int(c)) + "m";
}

inline void log(const std::string& msg = "", Color c = Color::RESET, bool bold = false) {
  std::cerr << color(c, bold) << msg << color(Color::RESET);
}

inline void logln(const std::string& msg = "", Color c = Color::RESET, bool bold = false) {
  log(msg, c, bold);
  std::cerr << std::endl;
}

inline void lnlog(const std::string& msg = "", Color c = Color::RESET, bool bold = false) {
  std::cerr << std::endl;
  log(msg, c, bold);
}

inline std::vector<std::function<void()>>* tests() {
  static std::vector<std::function<void()>> tests_;
  return &tests_;
}

template <typename F>
inline int add_test(F f) {
  tests()->push_back(f);
  return 0;
}

template <typename T>
void run_test(T t, const char* g, const char* n) {
  const std::string test_name = std::string(g) + "/" + n;

  log("Running " + test_name + ": ", Color::CYAN, true);

  error = false;
  try {
    t();
  } catch(...) {
    logln();
    logln(test_name + " throws an exception.");
  }

  if (!error) {
    logln("Test case successfully finished.", Color::GREEN);
  } else {
    logln("There is error in the test case.", Color::RED);
  }

  logln();
}

#define RUN(g, n) fn::test::run_test(TEST_NAME(g, n), #g, #n);

void run_all_tests() {
  for (auto& t : *tests()) {
    t();
  }
}

}  // namespace test
}  // namespace fn

#endif  // FUNC_TEST_TEST_H_
