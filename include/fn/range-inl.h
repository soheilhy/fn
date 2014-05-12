#ifndef FUNC_RANGE_INL_H_
#define FUNC_RANGE_INL_H_

#include <algorithm>

namespace fn {

template <typename T>
Range<T>::Range(T from, T to, int step) : from_(from), to_(to), step_(step) {}

template <typename T>
typename Range<T>::Iterator Range<T>::begin() const {
  return Iterator(this, from_);
}

template <typename T>
typename Range<T>::Iterator Range<T>::end() const {
  if (empty()) {
    return Iterator(this, from_);
  }

  return Iterator(this, to_);
}

template <typename T>
bool Range<T>::empty() const {
  if (step_ > 0 && to_ < from_) {
    return true;
  }

  if (step_ < 0 && to_ > from_) {
    return true;
  }

  return from_ == to_;
}

template <typename T>
size_t Range<T>::size() const {
  if (step_ > 0 && to_ < from_) {
    return 0;
  }

  if (step_ < 0 && to_ > from_) {
    return 0;
  }

  return std::abs((from_ - to_) / step_);
}

template <typename T>
Range<T>::Iterator::Iterator(const Range<T>* range, T val)
    : range_(range), val_(val) {}

template <typename T>
typename Range<T>::Iterator& Range<T>::Iterator::operator++() {
  if (range_->step_ > 0) {
    val_ = std::min(range_->to_, val_ + range_->step_);
  } else {
    val_ = std::max(range_->to_, val_ + range_->step_);
  }
  return *this;
}

template <typename T>
typename Range<T>::Iterator Range<T>::Iterator::operator++(int) {
  auto temp = *this;
  if (range_->step_ > 0) {
    val_ = std::min(range_->to_, val_ + range_->step_);
  } else {
    val_ = std::max(range_->to_, val_ + range_->step_);
  }
  return temp;
}

template <typename T>
bool Range<T>::Iterator::operator==(const Iterator& that) const {
  return val_ == that.val_;
}

template <typename T>
bool Range<T>::Iterator::operator!=(const Iterator& that) const {
  return !(*this == that);
}

template <typename T>
const T& Range<T>::Iterator::operator*() const {
  return val_;
}

template <typename T>
const T& Range<T>::Iterator::operator->() const {
  return val_;
}

}  // namespace fn

#endif  // FUNC_RANGE_INL_H_

