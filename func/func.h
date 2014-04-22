

#ifndef FUNC_INCLUDE_FUNC_H_
#define FUNC_INCLUDE_FUNC_H_

#include <functional>
#include <type_traits>
#include <memory>

namespace func {

enum class FuncType {
  FILTER,
  FOLD_LEFT,
  MAP,
};

template <template <typename> class C, typename E, typename P = void,
          typename F = std::function<void()>, FuncType type = FuncType::FILTER>
class View {
 private:
  using CPtr = std::unique_ptr<C<E>>;

  // Solely for type deduction.
  static E* e_;

 public:
  View(const C<E> &c) : container_(new C<E>(c)), evaluated_(true) {}
  View(C<E> &&c) : container_(new C<E>(std::move(c))), evaluated_(true) {}
  View(P* p, F f)
      : container_(nullptr), evaluated_(false), parent_(p), func_(f) {}

  View(const View&) = delete;
  View& operator=(const View&) = delete;

  View(View&&) = default;
  View& operator=(View&&) = default;

  /** Filters the content of this view using the given function. */
  template <typename G>
  View<C, E, View, G> filter(G g) {
    return View<C, E, View, G>(this, g);
  }

  /** Maps the content of this view using the given function. */
  template <typename G>
  auto map(G g)
      -> View<C, typename std::decay<decltype(g(*e_))>::type, View, G,
              FuncType::MAP> {
    return View<C, typename std::decay<decltype(g(*e_))>::type, View, G,
                FuncType::MAP>(this, g);
  }

  template <typename T, typename G>
  T fold_left(T init, G g) {
    evaluate([&init, &g](const E& e) { init = g(init, e); });
    return std::move(init);
  }

  template <typename G>
  E reduce(G g) {
    bool first = true;
    E init;
    evaluate([&](const E& e) {
      if (first) {
        init = e;
        first = false;
        return;
      }
      init = g(init, e);
    });
    return std::move(init);
  }

  /** Whether the result of this view is already calculated. */
  bool is_evaluated() { return !!container_; }

  /** Evaluates the view. */
  C<E> evaluate();

  /** For converting the view to an actual container. */
  operator C<E>();

 private:
  template <typename G,
            typename std::enable_if<!(sizeof(G) && std::is_void<P>::value), int>::type = 0>
  void evaluate(G g);

  template <typename G,
            typename std::enable_if<(sizeof(G) && std::is_void<P>::value), int>::type = 0>
  void evaluate(G g);

  // The view is either materialized or not. If materalized container_ would
  // point to the container holding the actual data.
  CPtr container_;
  bool evaluated_;

  // Or otherwise, we have a step information indicating the parent view, and
  // the function we need to apply on it.
  P* parent_;
  F func_;

  template <template <typename> class CF, typename EF, typename PF, typename FF,
          FuncType TF>
  friend class View;
};

}  // namespace func

#include "func/func-inl.h"

#endif  // FUNC_INCLUDE_FUNC_H_

