#ifndef FUNC_INCLUDE_FUNC_H_
#define FUNC_INCLUDE_FUNC_H_

#include <cassert>

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <unordered_map>

#include "func/details.h"

namespace func {

// View logically represents a collection C of elements of type E. View provides
// functional programming primitives, such as filter, map, and reduce to name a
// few.
//
// To create a view, one needs to call func::_():
//
//   auto view = func::_(std::vector<int>({1, 2, 3, 4});
//
// View methods either return a final value, or create another view. Those
// created view are all lazily evaluated to minimize the overhead of
// intermediary collections.
//
// To evaluate a view, you can either call the evaluate() method or convert the
// view to C<E>.
//
// View is immutable, and it is safe to share them among threads.
template <template <typename...> class C, typename E, typename P = void*,
          typename F = std::function<void()>, FuncType t = FuncType::FILTER>
class View {
 public:
  using Element = E;
  using CPtr = std::unique_ptr<C<E>>;

  template <typename G>
  using FView = View<C, E, View, G>;

  template <typename MP, typename G>
  using MView = View<C, typename std::decay<MP>::type, View, G,
              FuncType::MAP>;

  // Use func::_ instead. These constructors are not really public.
  View(const C<E>& c, Private);
  View(C<E>&& c, Private);
  View(const P& p, F f, Private);
  View(const P& p, F f, E data, uint64_t metadata, Private);

  // Copyable.
  View(const View&);
  View& operator=(const View&);

  // Movable.
  View(View&&) = default;
  View& operator=(View&&) = default;

  ~View() {}

  // Filters the content of this view using the given function.
  template <typename G>
  FView<G> filter(G g);

  // Maps the content of this view using the given function.
  template <typename G>
  auto map(G g) -> MView<decltype(g(*(E*) nullptr)), G>;

  // Folds the content of this view from left. Uses the given initial value.
  template <typename T, typename G>
  T fold_left(T init, G g);

  // Reduces the content of this view from left.
  template <typename G>
  E reduce(G g);

  // Calls g for each element in the view.
  template <typename G>
  void for_each(G g);

  // Skips element until g returns true.
  template <typename G>
  View<C, E, View, G, FuncType::SKIP> skip_until(G g);

  // Keeps elements while g returns true.
  template <typename G>
  View<C, E, View, G, FuncType::KEEP> keep_while(G g);

  // Drop the first n elements of the view.
  View<C, E, View, std::function<bool(const E&)>, FuncType::SKIP> drop(
      size_t n = 1);

  // Zips this view with another view.
  template <template <typename...> class C2, typename E2, typename P2,
            typename F2, FuncType t2>
  View<C, std::pair<E, E2>,
       std::pair<View<C, E, P, F, t>, View<C2, E2, P2, F2, t2>>,
       std::function<void()>, FuncType::ZIP>
      zip(const View<C2, E2, P2, F2, t2>& that);

  // Produces the sum of elements in the view.
  E sum();

  // Produces the product of elements in the view.
  E product();

  // Returns the first element in the view.
  E first();

  // Returns the last element in the view.
  E last();

  // Returns true if the g returns true for all elements, otherwise returns
  // false.
  template <typename G>
  bool for_all(G g);

  // Whether the result of this view is already calculated.
  bool is_evaluated() { return !!container_; }

  // Evaluates the view.
  C<E> evaluate();

  // For converting the view to an actual container.
  operator C<E>();

  // Syntactic sugar for map().
  template <typename G>
  auto operator*(G g) -> MView<decltype(g(*(E*) nullptr)), G>;

  // Syntactic sugar for filter().
  template <typename G>
  FView<G> operator%(G g);

  // Syntactic sugar for reduce().
  template <typename G>
  E operator/(G g);

  // Syntactic sugar for for_each().
  template <typename G>
  View& operator>>(G g);

  template <template <typename...> class C2, typename E2, typename P2,
            typename F2, FuncType t2>
  View<C, std::pair<E, E2>,
       std::pair<View<C, E, P, F, t>, View<C2, E2, P2, F2, t2>>,
       std::function<void()>, FuncType::ZIP>
      operator+(const View<C2, E2, P2, F2, t2>& that);

 private:
  bool is_filter() { return t == FuncType::FILTER; }

  template <typename G,
            typename std::enable_if<sizeof(G) && std::is_same<void*, P>::value,
                                    int>::type = 0>
  void do_evaluate(G g);

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == FuncType::FILTER,
                            int>::type = 0>
  void do_evaluate(G g);

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == FuncType::ZIP,
                            int>::type = 0>
  void do_evaluate(G g);

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == FuncType::SKIP,
                            int>::type = 0>
  void do_evaluate(G g);

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == FuncType::KEEP,
                            int>::type = 0>
  void do_evaluate(G g);

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == FuncType::MAP,
                            int>::type = 0>
  void do_evaluate(G g);

  template <typename G, typename std::enable_if<
                            sizeof(G) && !std::is_same<void*, P>::value &&
                                t == FuncType::FOLD_LEFT,
                            int>::type = 0>
  void do_evaluate(G g);

  // The view is either materialized or not. If materalized container_ would
  // point to the container holding the actual data.
  CPtr container_;

  // Or otherwise, we have a step information indicating the parent view, and
  // the function we need to apply on it.
  P parent_;
  F func_;

  // These are private data used by func_, if it requires to store a context.
  E data_;
  uint64_t metadata_;

  template <template <typename...> class CF, typename EF, typename PF,
            typename FF, FuncType TF>
  friend class View;
};

// Creates a view of the given collection.
template <template <typename...> class C, typename E>
View<C, E> _(C<E>&& c);

template <template <typename...> class C, typename E>
View<C, E> _(const C<E>& c);

template <typename K, typename V>
View<std::vector, std::pair<K, V>> _(const std::unordered_map<K, V>& l);

#if defined __cplusplus && __cplusplus > 201103L

#define _$ [&](auto _1)
#define _$$ [&](auto _1, auto _2)

#endif

}  // namespace func

#include "func/func-inl.h"

#endif  // FUNC_INCLUDE_FUNC_H_

