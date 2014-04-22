#ifndef FUNC_FUNC_INL_H_
#define FUNC_FUNC_INL_H_

#include <cassert>

namespace func {

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G, typename std::enable_if<
                          (sizeof(G) && std::is_void<P>::value), int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  assert(is_evaluated() && "Cannot evaluate a view without a parent.");

  for (const auto& e : *container_) {
    g(e);
  }
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G, typename std::enable_if<
                          !(sizeof(G) && std::is_void<P>::value), int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  if (is_evaluated()) {
    for (const auto& e : *container_) {
      g(e);
    }

    return;
  }

  using PE = typename std::decay<decltype(*P::e_)>::type;
  switch (t) {
    case FuncType::FILTER: {
      do_filter<PE>(g);
      break;
    }
    case FuncType::SKIP: {
      do_skip<PE>(g);
      break;
    }
    default: {
      parent_->evaluate([this, &g](const PE& e) { g(func_(e)); });
    }
  }
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
C<E> View<C, E, P, F, t>::evaluate() {
  if (is_evaluated()) {
    return *container_;
  }

  C<E> c;
  evaluate([&c](const E& e) { c.push_back(e); });
  return std::move(c);
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
View<C, E, P, F, t>::operator C<E>() {
  return std::move(evaluate());
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename PE, typename G>
void View<C, E, P, F, t>::do_filter(G g) {
  assert(t == FuncType::FILTER && "Filering on a view that is not a filter.");

  parent_->evaluate([this, &g](const PE& e) {
    if (!func_(e)) {
      return;
    }

    g(e);
  });
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename PE, typename G>
void View<C, E, P, F, t>::do_skip(G g) {
  assert(t == FuncType::SKIP && "Skiping on a view that is not a skip.");

  bool passed = false;
  parent_->evaluate([this, &g, &passed](const PE& e) {
    if (!passed && !func_(e)) {
      return;
    }

    passed = true;
    g(e);
  });
}

}  // namespace func

#endif  // FUNC_FUNC_INL_H_

