#ifndef FUNC_FUNC_INL_H_
#define FUNC_FUNC_INL_H_

#include <cassert>

namespace func {

template <template <typename> class C, typename E, typename P, typename F,
          FuncType type>
template <typename G, typename std::enable_if<
                          (sizeof(G) && std::is_void<P>::value), int>::type>
void View<C, E, P, F, type>::evaluate(G g) {
  assert(is_evaluated() && "Cannot evaluate a view without a parent.");

  for (const auto& e : *container_) {
    g(e);
  }
}

template <template <typename> class C, typename E, typename P, typename F,
          FuncType type>
template <typename G, typename std::enable_if<
                          !(sizeof(G) && std::is_void<P>::value), int>::type>
void View<C, E, P, F, type>::evaluate(G g) {
  if (is_evaluated()) {
    for (const auto& e : *container_) {
      g(e);
    }

    return;
  }

  using PE = typename std::decay<decltype(*P::e_)>::type;
  parent_->evaluate([this, &g](const PE& e) {
    if (type == FuncType::FILTER) {
      if (!func_(e)) {
        return;
      }

      g(e);
      return;
    }

    g(func_(e));
  });
}

template <template <typename> class C, typename E, typename P, typename F,
          FuncType type>
C<E> View<C, E, P, F, type>::evaluate() {
  if (is_evaluated()) {
    return *container_;
  }

  C<E> c;
  evaluate([&c](const E& e) { c.push_back(e); });
  return std::move(c);
}

template <template <typename> class C, typename E, typename P, typename F,
          FuncType type>
View<C, E, P, F, type>::operator C<E>() {
  return std::move(evaluate());
}

}  // namespace func

#endif  // FUNC_FUNC_INL_H_

