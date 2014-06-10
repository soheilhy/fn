// Copyright 2014, The Project fn Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.

#ifndef FN_FN_H_
#define FN_FN_H_

#include <cassert>

#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "fn/details.h"
#include "fn/range.h"

namespace fn {

// View logically represents a collection C of elements of type E. View provides
// functional programming primitives, such as filter, map, and reduce to name a
// few.
//
// To create a view, one needs to call fn::_():
//
//   auto view = fn::_({1, 2, 3, 4});
//
// View methods either return a final value, or create another view. Those
// created view are all lazily evaluated to minimize the overhead of
// intermediary collections.
//
// To evaluate a view, you can either call the evaluate() method or convert the
// view to C<E>.
//
// View is almost immutable (it is modified when moved), and it is safe to share
// them among threads. Just make sure you don't move it when shared between
// threads.
template <template <typename...> class C, typename E,
          template <typename...> class R = fn::details::Copy,
          typename P = void*, typename F = std::function<void()>,
          fn::details::FuncType t = fn::details::FuncType::FILTER>
class View {
 public:
  using Element = E;
  using Container = C<E>;
  using CPtr = R<C<E>>;
  using PView = P;

  template <typename G>
  using FView = View<C, E, R, View, G>;

  template <typename MP, typename G>
  using MView = View<C, typename std::decay<MP>::type, R, View, G,
                     fn::details::FuncType::MAP>;

  using Iterator = fn::details::ViewIterator<View, PView, t>;

  // For compability with stl. Never used internally.
  using iterator = Iterator;
  using const_iterator = Iterator;
  using value_type = Element;

  static const fn::details::FuncType func_type = t;

  // Use fn::_ instead. These constructors are not technically public.
  View(const C<E>& c, fn::details::Private);
  View(C<E>&& c, fn::details::Private);
  View(const P& p, F f, fn::details::Private);

  // Copyable, not copy-assignable.
  View(const View&);
  View& operator=(const View&) = delete;

  // Movable (will be copied), not move-assignable.
  View(View&&) = default;
  View& operator=(View&&) = delete;

  ~View() {}

  // Filters the content of this view using the given function.
  template <typename G>
  FView<G> filter(G g) const;

  // Maps the content of this view using the given function.
  template <typename G>
  auto map(G g) -> MView<decltype(g(*(E*) nullptr)), G> const;

  template <typename G>
  auto flat_map(G g)
      -> View<C, typename decltype(g(*(E*) nullptr))::value_type, R, View, G,
              fn::details::FuncType::FLAT_MAP> const;

  // Folds the content of this view from left. Uses the given initial value.
  template <typename T, typename G>
  T fold_left(T init, G g) const;

  // Reduces the content of this view from left.
  template <typename G>
  E reduce(G g) const;

  // Calls g for each element in the view.
  template <typename G>
  void for_each(G g) const;

  // Skips element until g returns true.
  template <typename G>
  View<C, E, R, View, G, fn::details::FuncType::SKIP> skip_until(G g) const;

  // Keeps elements while g returns true.
  template <typename G>
  View<C, E, R, View, G, fn::details::FuncType::KEEP> keep_while(G g) const;

  // Zips this view with another view.
  template <template <typename...> class C2, typename E2, template <typename...>
            class R2, typename P2, typename F2, fn::details::FuncType t2>
  View<C, std::pair<E, E2>, R,
       std::pair<View<C, E, R, P, F, t>, View<C2, E2, R2, P2, F2, t2>>,
       std::function<void()>, fn::details::FuncType::ZIP>
      zip(const View<C2, E2, R2, P2, F2, t2>& that) const;

  // Produces the sum of elements in the view.
  E sum() const;

  // Produces the product of elements in the view.
  E product() const;

  // Returns the first element in the view.
  E first() const;

  // Returns the last element in the view.
  E last() const;

  // Returns the minimum element in the view. Assumes that operator< is defined
  // for E.
  E min() const;

  // Returns the maximum element in the view. Assumes that operator< is defined
  // for E.
  E max() const;

  // Returns the number of elements in the view.
  size_t size() const;

  // Returns the size of the container (ie, source) stored in the root view.
  template <typename RP = P, typename std::enable_if<
                                 std::is_same<void*, RP>::value, int>::type = 0>
  size_t root_size() const { return container_->size(); }

  template <
      typename RP = P,
      typename std::enable_if<!std::is_same<void*, RP>::value, int>::type = 0>
  size_t root_size() const { return parent_.root_size(); }

  // Returns true if the g returns true for all elements, otherwise returns
  // false.
  template <typename G>
  bool for_all(G g) const;

  // Whether the result of this view is already calculated.
  bool is_evaluated() const { return !!container_; }

  // Evaluates the view.
  C<E> evaluate() const;

  // For converting the view to an actual container.
  explicit operator C<E>() const;

  // Returns the values in the view as a vector.
  std::vector<E> as_vector() const;

  // Returns the values in the view as a list.
  std::list<E> as_list() const;

  // Returns the values in the view as a deque.
  std::deque<E> as_deque() const;

  // Returns the values as a set.
  // Note: This is different than distinct(). Here we simply insert elements
  // into unordered_set, but in disctinct() we use std::unique. They have
  // different performance implications.
  std::unordered_set<E> as_set() const;

  // Returns the values in a sorted order using c as the comparator.
  template <typename Cmp>
  C<E> sort(Cmp c = std::less<E>()) const;

  // Returns distinct values using eq as the equality function.
  // Note: See as_set().
  template <typename Eq>
  C<E> distinct(Eq eq = std::equal_to<E>()) const;

  // Returns the values in the view as a map.
  template <typename K, typename V,
            typename std::enable_if<
                sizeof(K) && fn::details::is_pair<E>::value, int>::type = 0>
  std::unordered_map<K, V> as_map() const;

  // Evaluates the view and append the entreies to c.
  template <template <typename...> class EC>
  void evaluate(EC<E>* c) const;

  // Evaluates the view and insert the pais in a map.
  template <typename K, typename V,
            typename std::enable_if<
                sizeof(K) && fn::details::is_pair<E>::value, int>::type = 0>
  void evaluate(std::unordered_map<K, V>* m) const;

  // Syntactic sugar for map().
  template <typename G>
  auto operator*(G g) -> MView<decltype(g(*(E*) nullptr)), G> const;

  // Syntactic sugar for filter().
  template <typename G>
  FView<G> operator%(G g) const;

  // Syntactic sugar for reduce().
  template <typename G>
  E operator/(G g) const;

  // Syntactic sugar for for_each().
  template <typename G>
  const View& operator>>(G g) const;

  // Syntactic sugar for in-place evaluate.
  template <template <typename...> class EC>
  const View& operator>>(EC<E>* container) const;

  template <template <typename...> class C2, typename E2, template <typename...>
            class R2, typename P2, typename F2, fn::details::FuncType t2>
  View<C, std::pair<E, E2>, R,
       std::pair<View<C, E, R, P, F, t>, View<C2, E2, R2, P2, F2, t2>>,
       std::function<void()>, fn::details::FuncType::ZIP>
      operator+(const View<C2, E2, R2, P2, F2, t2>& that) const;

  Iterator begin() const;

  template <typename T = int,
            typename std::enable_if<
                sizeof(T) && std::is_same<void*, P>::value, int>::type = 0>
  Iterator end() const;

  template <typename T = int, typename std::enable_if<
                                  sizeof(T) && !std::is_same<void*, P>::value &&
                                      t != fn::details::FuncType::ZIP,
                                  int>::type = 0>
  Iterator end() const;

  template <typename T = int, typename std::enable_if<
                                  sizeof(T) && !std::is_same<void*, P>::value &&
                                      t == fn::details::FuncType::ZIP,
                                  int>::type = 0>
  Iterator end() const;

 private:
  template <typename G,
            typename std::enable_if<sizeof(G) && std::is_same<void*, P>::value,
                                    int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::FILTER,
                            int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::ZIP,
                            int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::SKIP,
                            int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::KEEP,
                            int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::MAP,
                            int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::FLAT_MAP,
                            int>::type = 0>
  void do_evaluate(G g) const;

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == fn::details::FuncType::FOLD_LEFT,
                            int>::type = 0>
  void do_evaluate(G g) const;

  // The view is either materialized or not. If materalized container_ would
  // point to the container holding the actual data.
  CPtr container_;

  // Or otherwise, we have a step information indicating the parent view, and
  // the function we need to apply on it.
  P parent_;
  F func_;

  template <template <typename...> class CF, typename EF, template <typename...>
            class RF, typename PF, typename FF, fn::details::FuncType tf>
  friend class View;

  template <typename IV, typename IP, fn::details::FuncType it>
  friend class fn::details::ViewIterator;
};

// Creates a view of the given collection.
template <template <typename...> class C, typename E>
View<C, E> _(C<E>&& c);

template <template <typename...> class C, typename E>
View<C, E> _(const C<E>& c);

template <template <typename...> class C, typename E>
View<C, E, fn::details::Ref> _(const C<E>* c);

template <typename E>
View<std::vector, E> _(const std::initializer_list<E>& l);

template <typename K, typename V>
View<std::vector, std::pair<K, V>> _(const std::unordered_map<K, V>& l);

#define FN_CXX1Y (__cplusplus && __cplusplus > 201103L)

#if FN_CXX1Y
#define _$ [&](auto _1)
#define _$$ [&](auto _1, auto _2)
#endif

}  // namespace fn

#include "fn/fn-inl.h"

#endif  // FN_FN_H_

