#include <algorithm>
#include <vector>
#include <unordered_map>
#include <utility>

#include "fn/range.h"
#include "fn/fn.h"
#include "test/test.h"

using std::make_pair;
using std::pair;
using std::vector;
using std::unordered_map;

using fn::_;
using fn::range;

TEST(Basic, Map) {
  int last = 0;
  _({1, 2, 3, 4, 5}).map([](int i) { return i * 2; }).for_each([&](int i) {
    EXPECT_EQ(2 * (last + 1), i, "Wrong number produced");
    last++;
  });

  EXPECT_EQ(5, last, "Wrong number of elements visited.");
}

TEST(Basic, FlatMap) {
  auto v = _({1, 2, 3, 4, 5})
               .flat_map([](int i) { return std::vector<int>{i, i * 10}; })
               .as_vector();

  EXPECT_EQ(size_t(10), v.size(), "The vector should contain 10 elements.");
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(i + 1, v[i * 2], "");
    EXPECT_EQ((i + 1) * 10, v[i * 2 + 1], "");
  }
}

TEST(Basic, Filter) {
  vector<int> f = _(vector<int>({1, 2, 3, 4}))
                           .filter([](int i) { return i % 2 == 0; });
  EXPECT_EQ(2, f.size(), "There are 2 even numbers in the view.");
  EXPECT_EQ(2, f[0], "The first one should be 2.");
  EXPECT_EQ(4, f[1], "The second one should be 4.");
}

TEST(Basic, FoldLeft) {
  auto max = _({4, 5, 6, 3, 2, 1})
                 .fold_left(-1, [](int m, int i) { return std::max(m, i); });
  EXPECT_EQ(6, max, "Incorrect value returned.");
}

TEST(Basic, Reduce) {
  auto max = _({-4, -5, -6, -3, -2, -7})
                 .reduce([](int m, int i) { return std::max(m, i); });
  EXPECT_EQ(-2, max, "Incorrect value returned.");
}

TEST(Basic, ForAll) {
  auto v = _({1, 2, 3, 4, 5});

  auto all_pos = v.for_all([](int i) { return i > 0; });
  EXPECT_TRUE(all_pos, "All elements of the view are positive.");

  auto all_even = v.for_all([](int i) { return i % 2 == 0; });
  EXPECT_FALSE(all_even, "View contains odd numbers.");
}

TEST(Basic, Zip) {
  auto lst = vector<int>({0, 1, 2, 3, 4});
  auto zip = _(&lst).zip(_(lst));
  auto zv = zip.evaluate();
  // zip should be {(0, 0), (1, 1), ..., (4, 4)}.

  EXPECT_EQ(lst.size(), zip.size(),
            "Zipped vector should be of the same size of the original vector.");

  for (auto i = 1; i < 5; i++) {
    auto f = zv[i].first, s = zv[i].second;
    EXPECT_EQ(i, f, "Incorrect first element.");
    EXPECT_EQ(i, s, "Incorrect second element.");
    EXPECT_EQ(f, s, "First and second elements should be equal.");
  }

  size_t count = 0;
  for (auto e : zip) {
    auto f = e.first, s = e.second;
    EXPECT_EQ(f, s, "First and second elements should be equal.");
    count++;
  }
  EXPECT_EQ(size_t(5), count, "There should be 5 elements zipped.");
}

TEST(Basic, First) {
  auto first =
      _({1, 2, 3, 4, 5}).filter([](int i) { return i % 2 == 0; }).first();
  EXPECT_EQ(2, first, "The first even number should be 2.");
}

TEST(Basic, Sum) {
  auto lst = {0, 1, 2, 3, 4};

  auto sum = 0;
  for (auto i : lst) {
    sum += i;
  }

  EXPECT_EQ(sum, _(&lst).sum(), "Wrong some for the view.");
}

TEST(Basic, Product) {
  auto lst = {0, 1, 2, 3, 4};

  auto prod = 1;
  for (auto i : lst) {
    prod *= i;
  }

  EXPECT_EQ(prod, _(&lst).product(), "Wrong some for the view.");
}

TEST(Basic, SkipUntil) {
  vector<int> v = _({0, 1, 2, 1, 2}).skip_until([](int i) { return i >= 2; });
  EXPECT_EQ(3, v.size(), "View should contain only 3 elements.");
  EXPECT_EQ(2, v[0], "First element should be 2");
  EXPECT_EQ(1, v[1], "Second element should be 1");
  EXPECT_EQ(2, v[2], "Third element should be 2");
}

TEST(Basic, KeepWhile) {
  vector<int> v = _({0, 1, 2, 1, 2}).keep_while([](int i) { return i < 2; });
  EXPECT_EQ(2, v.size(), "View should contain only 2 elements.");
  EXPECT_EQ(0, v[0], "First element should be 0");
  EXPECT_EQ(1, v[1], "Second element should be 1");
}

TEST(Basic, FirstLast) {
  auto v = _({1, 2, 3, 4, 5}).filter([](int i) { return i % 2 != 0; });
  EXPECT_EQ(3, v.size(), "View contains 3 odd elements.");
}

TEST(Baisc, Operators) {
  auto max1 = _({1, 2, 3, 4, 5}) * [](int i) { return i < 4 ? i * 10 : i; }
                                 % [](int i) { return i % 2 == 0; }
                                 / [](int m, int i) { return std::max(m, i); };

  EXPECT_EQ(30, max1, "The result should be 30.");

  auto max2 = _({1, 2, 3, 4, 5}) % [](int i) { return i % 2 == 0; }
                                 * [](int i) { return i < 4 ? i * 10 : i; }
                                 / [](int m, int i) { return std::max(m, i); };

  EXPECT_EQ(20, max2, "The result should be 20.");

}

TEST(Basic, InPlaceEvaluate) {
  vector<int> results;

  auto validate_and_reset_results = [&]() {
    EXPECT_EQ(size_t(3), results.size(),
              "The results vector is not correctly filled.");
    EXPECT_EQ(2, results[0], "");
    EXPECT_EQ(1, results[1], "");
    EXPECT_EQ(2, results[2], "");

    results.clear();
  };

  // Container.
  _({0, 1, 2, 1, 2}).skip_until([](int i) { return i > 1; }).evaluate(&results);
  validate_and_reset_results();

  // Syntax sugar.
  _({0, 1, 2, 1, 2}).skip_until([](int i) { return i > 1; }) >> &results;
  validate_and_reset_results();
}

TEST(Basic, InPlaceEvaluateMap) {
  unordered_map<int, int> m{{1, 1}, {2, 2}, {3, 3}};
  _(m).map([](const pair<int, int>& p) {
    return make_pair(p.first * 4, p.second + 1);
  }).evaluate(&m);

  EXPECT_EQ(size_t(6), m.size(),
            "Expecting 6 elements. Maybe the map is overwritten.");
  EXPECT_EQ(1, m[1], "Overwritten.");
  EXPECT_EQ(2, m[2], "Overwritten.");
  EXPECT_EQ(3, m[3], "Overwritten.");
  EXPECT_EQ(2, m[4], "Incorrectly mapped.");
  EXPECT_EQ(3, m[8], "Incorrectly mapped.");
  EXPECT_EQ(4, m[12], "Incorrectly mapped.");
}

TEST(Basic, Iterators) {
  auto r = _({1, 2, 3, 4, 5});

  size_t count = 0;
  size_t sum = 0;
  for (auto i : r % [](int i) { return i % 2 == 0; }) {
    count++;
    sum += i;
  }
  EXPECT_EQ(size_t(2), count, "There are only two even numbers in the view.");
  EXPECT_EQ(size_t(2 + 4), sum, "And their sum should be 6.");
}

// A vector that can't be copied.
template <typename T>
class V : public vector<T> {
 public:
  V() = default;
  V(const V&) = delete;
  V& operator=(const V&) = delete;
};

TEST(Basic, Ref) {
  V<int> v;
  v.push_back(1);
  v.push_back(2);

  auto r = _(&v).map([](int i) { return i * 2; }).as_vector();
  EXPECT_EQ(size_t(2), r.size(), "Expected two items in the results.");
  EXPECT_EQ(2, r[0], "");
  EXPECT_EQ(4, r[1], "");
}

TEST(Basic, Range) {
  auto r = range(1, 1);
  EXPECT_TRUE(r.empty(), "The range should be empty.");
  for (auto i : r) {
    EXPECT_FALSE(i && true, "Should never be ran for an empty range.");
  }

  r = range(2, 3, -1);
  EXPECT_TRUE(r.empty(), "The range should be empty.");
  for (auto i : r) {
    EXPECT_FALSE(i && true, "Should never be ran for an empty range.");
  }

  r = range(1, 3);
  EXPECT_EQ(size_t(2), r.size(), "Range should have two elements.");

  auto sum = 0;
  auto count = size_t(0);
  for (auto i : r) {
    sum += i;
    count++;
  }

  EXPECT_EQ(r.size(), count, "The loop didn't ran correctly.");
  EXPECT_EQ(1 + 2, sum, "Incorrect sum. The loop didn't run correctly.");


  r = range(3, 1, -1);
  EXPECT_EQ(size_t(2), r.size(), "Range should have two elements.");

  sum = 0;
  count = 0;
  for (auto i : r) {
    sum += i;
    count++;
  }

  EXPECT_EQ(r.size(), count, "The loop didn't ran correctly.");
  EXPECT_EQ(3 + 2, sum, "Incorrect sum. The loop didn't run correctly.");
}

TEST(Range, Functional) {
  vector<int> v = _(range(1, 2)).map([](int i) { return i * 2; }).as_vector();
  EXPECT_EQ(size_t(1), v.size(), "There should be only elements in the vector");
  EXPECT_EQ(2, v[0], "The first element is not correctly mapped.");
}

int main() {
  fn::test::run_all_tests();
}

