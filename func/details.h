#ifndef FUNC_INCLUDE_PRIVATE_H_
#define FUNC_INCLUDE_PRIVATE_H_

#include <memory>

namespace func {

enum class FuncType {
  FILTER,
  FOLD_LEFT,
  MAP,
  SKIP,
  ZIP,
};

class Private;

template <template <typename...> class C, typename T>
struct ContainerWrapper {
  C<T> data;
};

template <typename T>
struct Copy {
  T t;
};

template <typename T>
struct Pointer {
  T* t;
};

// Container policies.
template <typename T>
using RefCP = ContainerWrapper<Pointer, T>;

template <typename T>
using SharedPtrCP = ContainerWrapper<std::shared_ptr, T>;

template <typename T>
using CopyCP = ContainerWrapper<Copy, T>;

template <typename T>
class Optional {
 public:
  Optional(const T& t) : is_set_(true) {
    new (storage_) T(t);
  }

  Optional(T&& t) : is_set_(true) {
    new (storage_) T(std::move(t));
  }

  Optional(const Optional&) = default;
  Optional& operator=(const Optional&) = default;

  ~Optional() {
    if (!is_set()) {
      return;
    }

    this->operator*().~T();
  }

  T* operator->() {
    if (!is_set()) {
      return nullptr;
    }

    return reinterpret_cast<T*>(storage_);
  }

  T& operator*() {
    assert(is_set() && "Dereferencing optional when it's not set.");
    return *this->operator->();
  }

  bool is_set() { return is_set_; }

 private:
  bool is_set_;
  char storage_[sizeof(T)];
};

}  // namespace func

#endif  // FUNC_INCLUDE_PRIVATE_H_

