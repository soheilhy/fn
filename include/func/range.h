#ifndef FUNC_RANGE_H_
#define FUNC_RANGE_H_

namespace func {

// A simple iteratable range class.
template <typename T = int>
class Range {
 public:
  class Iterator {
   public:
    Iterator(const Range* range, T val);

    T& operator*();
    const T& operator*() const;

    Iterator& operator++();

    bool operator==(const Iterator& that) const;
    bool operator!=(const Iterator& that) const;

   private:
    const Range* range_;
    T val_;
  };

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

}  // namespace func

#include "func/range-inl.h"

#endif  // FUNC_RANGE_H_

