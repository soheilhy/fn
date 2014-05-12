#_fn_#
_fn_ is a functional programming library for C++11. It has a concise
syntax, is header-only and has no dependencies. It works with `gcc-4.8+`
and `clang-3.4+`.

##tldr; Summary##
_fn_ provides a lazy evaluated stream, called View, (similar to Scala's
`view`) on which you can apply the usual functional programming
paradigms, including but not limited to `filter`, `map`, `reduce`,
`zip`, `fold_left`, `flat_map` and `for_all`. View are iteratble.

Using _fn_, you can hack like this in C++:

**MAX** Find the maximum of elements in a container (e.g., vector, list,
...).
```c++
std::vector<int> v{1, 2, 3, 4, 5};
auto max = _(v).reduce([](int m, int i) { return std::max(m, i); });
// or simply by calling `max`:
auto max = _(v).max();
// or even prevent an extra copy by passing a pointer to the vector:
auto max = _(&v).max();
```

**EVENS** Find all even elements in a vector.
```c++
std::vector<int> evens = _(&v).filter([](int i) { return i % 2 == 0; });
// or, append them to an existing container:
_(&v).filter([](int i) { return i % 2 == 0; }) >> &evens;
// or, print them:
_(&v).filter([](int i) { return i % 2 == 0; }) >> [](int i) {
  printf("%d\n", i);
};
// or, use a fancy filter operator:
_(&v) % [](int i) { return i % 2 == 0; } >> [](int i) {
  printf("%d\n", i);
};
// or, iterate over them:
for (auto i : _(&v) % [](int i) { return i % 2 == 0; }) {
  printf("%d\n", i);
}
```

**XOR** Find the xor of all even elements in a vector.
```c++
std::vector<int> v{1, 2, 3, 4, 5};
auto r = _(&v).filter([](int i) { return i % 2 == 0; })
              .reduce([](int m, int i) { return m ^ i; });
```

**WORD LEN COUNT** Count the number of words of each distinct length.
```C++
vector<string> words {"map", "fold", "filter", "reduce", "any"};
auto len_count =
    _(&words).map([](const string& w) { return w.size(); })
             .fold_left(map<int, int>(), [](map<int, int>&& cnt,
                                            size_t len) {
                cnt[len]++;
                return cnt;
              });

// And if your compiler is fully functional for C++14, you can
// write a lot less using _$ and _$$ macros. These macros rely on
// C++14's automatic type deduction for lambda parameters.
auto len_count =
    _(&words).map(_$ { return _1.size(); })
             .fold_left(map<int, int>(), _$$ {
                _1[_2]++;
                return _1;
              });
```

**[Euler #1][1]** Find the sum of all the multiples of 3 or 5 below
1000.
```c++
// Here we use fn::range which is convenient, efficient, and iteratable.
int s = _(range(1, 1000))
            .filter([](int i) { return i % 3 == 0 || i % 5 == 0; })
            .sum();
```

**[Euler #4][2]** Find the largest palindrome made from the product of
two 3-digit numbers.
```c++
int max = _(range(999, 99, -1))
              .flat_map([](int i) {
                 return _(range(999, i, -1))
                     .map([i](int j) { return i * j; });
               })
              .filter([](int i) {
                 auto s = std::to_string(i);
                 return std::equal(s.begin(), s.end(), s.rbegin());
               })
              .max();
```
##How to use##
Just add _fn_'s `include` directory to your C++ include directories:
```
CXX -std=c++11 -I ${FN_HOME}/include/ ...
```
That's all. It's a header only library with **no** dependecies, **no**
configuration, **no** installation, etc.

##Introduction##
C++11 come with awesome functional programming concepts but the syntax
is not as pleasant as what you get in Scala or Haskell. The goal of _fn_
is to provide a lightweight library with a simple and sweet syntax for
common functional programming paradigms.

_fn_ provides two main concepts `View` (inspired by Scala's view) and
`Range` (inspired by Python's xrange).

###View###
`View` simply embeds a container, or anything iteratble. To create a
view, you need to call `fn::_`:

```c++
#include "fn/fn.h"

using fn::_;
using fn::range;

int main() {
  auto view = _({1, 2, 3, 4});
  for (auto i : view) {
    ...
  }
}
```

You can transform a view to another view using functions `map`,
`filter`, `flat_map`:
```c++
#include "fn/fn.h"

using fn::_;
using fn::range;

int main() {
  auto view = _({-1, 2, -3, 4});
  auto f = view.filter([](int i) { return i > 0; });
  auto m = view.map([](int i) { return i * 2; });
  auto mm = view.flat_map([](int i) { return _(range(0, i)); });
  ...
}
```

Note that these functions are lazily evaluated, meaning that they won't
be called unless you evaluate the view by calling the evaluate method or
converting the view to a container:
```c++
#include <deque>
#include <list>
#include <vector>

#include "fn/fn.h"

using std::vector;

using fn::_;
using fn::range;

int main() {
  auto view = _({-1, 2, -3, 4});
  auto f = view.filter([](int i) { return i > 0; });
  vector<int> v1 = f.evaluate();
  vector<int> v2 = f;
  auto l = f.as_list();
  auto d = f.as_deque();
  ...
}
```

Also, if you call methods that result in a value (e.g., `reduce`, `max`,
`sum`, `first`, and `for_each`), the view will be evaluated:
```c++
#include "fn/fn.h"

using fn::_;
using fn::range;

int main() {
  auto view = _({-1, 2, -3, 4});
  auto m = view.max();
  auto f = view.first();
  ...
}
```

Views are **immutable** and by default copy the container to pass to
them. You can save for that copy by passing a pointer to `fn::_` if
you're sure the pointer will remain valid:
```c++
#include "fn/fn.h"

using fn::_;
using fn::range;

int main() {
  std::vector<int> vec {-1, 2, -3, 4};
  auto view = _(&vec);
  ...
}
```

###Range###
Range is pretty similar to python's `xrange`. To create a range, one
calls `fn::range()`:
```
#include "fn/range.h"

using fn::range;

int main() {
  for (auto i : range(1, 10)) {
    ...
  }
}

```

You can also pass the range to `_` create a view:
```
#include "fn/fn.h"
#include "fn/range.h"

using fn::range;
using fn::_;

int main() {
  auto sum = _(range(1, 11)).sum();
  ...
}

```

##Helper macros for C++14##
If your compiler supports C++14, you can exploit automatic type
deduction for lambda parameters. Actually, _fn_ has two macros to help
you with that `_$` and `_$$` which respectively create lambdas of one
and two parameters. You can access the paramters as `_1` and `_2`.
```c++
auto a = _$ { return _1 * 2; }
auto b = _$$ { return std::max(_1, _2); }
```



  [1]: http://projecteuler.net/problem=1
  [2]: http://projecteuler.net/problem=4
