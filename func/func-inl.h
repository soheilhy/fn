#ifndef FUNC_FUNC_INL_H_
#define FUNC_FUNC_INL_H_

#include <cassert>

namespace func {

class Private {
 private:
  Private() {}

  template <template <typename...> class CF, typename EF, typename PF,
            typename FF, FuncType TF>
  friend class View;

  template <template <typename...> class CF, typename EF>
  friend typename View<CF, EF>::SelfPtr make_view(CF<EF>&& c);
};

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
View<C, E, P, F, t>::View(const C<E> &c, Private) : container_(new C<E>(c)) {}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
View<C, E, P, F, t>::View(C<E>&& c, Private)
    : container_(new C<E>(std::move(c))) {}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
View<C, E, P, F, t>::View(const PPtr& p, F f, Private)
    : container_(nullptr), parent_(p), func_(f) {
  assert(p != nullptr && "Parent is nullptr.");
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
View<C, E, P, F, t>::View(const PPtr& p, F f, E data, uint64_t metadata,
                          Private)
    : container_(nullptr),
      parent_(p),
      func_(f),
      data_(data),
      metadata_(metadata) {
  assert(p != nullptr && "Parent is nullptr.");
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G>
typename View<C, E, P, F, t>::template VPtr<C, E, View<C, E, P, F, t>, G>
View<C, E, P, F, t>::filter(G g) {
  using V = View<C, E, View, G>;
  return std::make_shared<V>(this->shared_from_this(), g, Private());
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G>
auto View<C, E, P, F, t>::map(G g)
    -> typename View<C, E, P, F, t>::template VPtr<
          C, typename std::decay<decltype(g(*(E*) nullptr))>::type, View, G,
          FuncType::MAP> {
  using V = View<C, typename std::decay<decltype(g(*(E*) nullptr))>::type, View,
                 G, FuncType::MAP>;
  return std::make_shared<V>(this->shared_from_this(), g, Private());
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename T, typename G>
T View<C, E, P, F, t>::fold_left(T init, G g) {
  evaluate([&init, &g](const E& e) { init = g(init, e); });
  return std::move(init);
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G>
E View<C, E, P, F, t>::reduce(G g) {
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

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G>
void View<C, E, P, F, t>::for_each(G g) {
  evaluate([&](const E& e) { g(e); });
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G>
typename View<C, E, P, F, t>::template VPtr<C, E, View<C, E, P, F, t>, G,
                                            FuncType::SKIP>
View<C, E, P, F, t>::skip_until(G g) {
  using V = View<C, E, View, G, FuncType::SKIP>;
  return std::make_shared<V>(this->shared_from_this(), g, Private());
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
typename View<C, E, P, F, t>::template VPtr<
    C, E, View<C, E, P, F, t>, std::function<bool(const E&)>, FuncType::SKIP>
View<C, E, P, F, t>::drop(size_t n) {
  using V = View<C, E, View, std::function<bool(const E&)>, FuncType::SKIP>;
  auto v =
      std::make_shared<V>(this->shared_from_this(),
                          std::function<bool(const E&)>(), n - 1, Private());

  auto f = [v](const E&) {
    if (v->data_ == 0) {
      return true;
    }

    v->data_--;
    return false;
  };

  v->func_ = f;
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <template <typename...> class C2, typename E2, typename P2,
          typename F2, FuncType t2>
typename View<C, E, P, F, t>::template VPtr<
    C, std::pair<E, E2>,
    std::pair<typename View<C, E, P, F, t>::template VPtr<C, E, P, F, t>,
              typename View<C, E, P, F, t>::template VPtr<C2, E2, P2, F2, t2>>,
    std::function<void()>, FuncType::ZIP>
View<C, E, P, F, t>::zip(const VPtr<C2, E2, P2, F2, t2>& that) {
  using PtrPair = std::pair<VPtr<C, E, P, F, t>, VPtr<C2, E2, P2, F2, t2>>;
  return std::make_shared<
      View<C, std::pair<E, E2>, PtrPair, std::function<void()>, FuncType::ZIP>>(
      std::make_shared<PtrPair>(std::make_pair(this->shared_from_this(), that)),
      [] {}, Private());
}

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
template <typename G,
          typename std::enable_if<
              sizeof(G) && !std::is_void<P>::value && t == FuncType::FILTER,
              int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  using PE = typename std::decay<typename P::Element>::type;

  parent_->evaluate([this, &g](const PE& e) {
    if (!func_(e)) {
      return;
    }

    g(e);
  });
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G,
          typename std::enable_if<
              sizeof(G) && !std::is_void<P>::value && t == FuncType::ZIP,
              int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  auto p1 = parent_->first->evaluate();
  auto itr = p1.begin();
  auto end = p1.end();

  using P2E =
      typename std::decay<typename P::second_type::element_type::Element>::type;
  parent_->second->evaluate([&g, &itr, &end](const P2E& e) {
    if (itr == end) {
      return;
    }

    g(std::make_pair(*itr, e));
    itr++;
  });
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G,
          typename std::enable_if<
              sizeof(G) && !std::is_void<P>::value && t == FuncType::SKIP,
              int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  using PE = typename std::decay<typename P::Element>::type;

  bool passed = false;
  parent_->evaluate([this, &g, &passed](const PE& e) {
    if (!passed && !func_(e)) {
      return;
    }

    passed = true;
    g(e);
  });
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G,
          typename std::enable_if<
              sizeof(G) && !std::is_void<P>::value && t == FuncType::MAP,
              int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  using PE = typename std::decay<typename P::Element>::type;

  parent_->evaluate([this, &g](const PE& e) { g(func_(e)); });
}

template <template <typename...> class C, typename E, typename P, typename F,
          FuncType t>
template <typename G,
          typename std::enable_if<
              sizeof(G) && !std::is_void<P>::value && t == FuncType::FOLD_LEFT,
              int>::type>
void View<C, E, P, F, t>::evaluate(G g) {
  using PE = typename std::decay<typename P::Element>::type;

  parent_->evaluate([this, &g](const PE& e) { g(func_(e)); });
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

template <template <typename...> class C, typename E>
typename View<C, E>::SelfPtr make_view(C<E>&& c) {
  return std::make_shared<View<C, E>>(std::forward<C<E>>(c),
                                      Private());
}

}  // namespace func

#endif  // FUNC_FUNC_INL_H_

