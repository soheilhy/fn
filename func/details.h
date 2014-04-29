#ifndef FUNC_INCLUDE_PRIVATE_H_
#define FUNC_INCLUDE_PRIVATE_H_

#include <memory>

namespace func {

enum class FuncType {
  FILTER,
  FOLD_LEFT,
  KEEP,
  MAP,
  SKIP,
  ZIP,
};

class Private;

template <typename T>
struct Copy {
  Copy() : p() {}
  Copy(const T& t) : p(new T(t)) {}
  Copy(T&& t) : p(new T(std::move(t))) {}

  Copy(const Copy& that) : p(that.p ? new T(*that.p) : nullptr) {}
  Copy(Copy&&) = default;

  const T& operator*() const { return *p; }
  const T* operator->() const { return p.get(); }

  bool operator!() const { return !bool(*this); }
  operator bool() const { return p != nullptr; }

  std::unique_ptr<const T> p;
};

template <typename T>
struct Ref {
  Ref() : p(nullptr) {}
  Ref(const T& t) : p(&t) {}

  const T& operator*() const { return *p; }
  const T* operator->() const { return p; }

  bool operator!() const { return !bool(*this); }
  operator bool() const { return p != nullptr; }

  const T* p;
};

}  // namespace func

#endif  // FUNC_INCLUDE_PRIVATE_H_

