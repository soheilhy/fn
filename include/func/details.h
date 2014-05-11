#ifndef FUNC_DETAILS_H_
#define FUNC_DETAILS_H_

#include <memory>
#include <tuple>

namespace func {
namespace details {

enum class FuncType {
  FILTER,
  FLAT_MAP,
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

template <typename>
struct is_pair {
  const static bool value = false;
};

template <typename F, typename S>
struct is_pair<std::pair<F, S>> {
  const static bool value = true;
};

}  // namespace details
}  // namespace func

#endif  // FUNC_DETAILS_H_

