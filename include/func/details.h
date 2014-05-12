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

template <typename View, typename PView = typename View::PView,
          FuncType ftype = View::func_type>
class ViewIterator;

template <typename View, typename PView>
class ViewIterator<View, PView, FuncType::MAP> : public std::iterator<
                                                     std::forward_iterator_tag,
                                                     typename View::Element> {
 public:
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view, ViewIterator<PView>(&view->parent_)) {}

  ViewIterator(const View* view, ViewIterator<PView>&& iter)
      : view_(view), iter_(std::move(iter)) {}

  ViewIterator& operator++() {
    ++iter_;
    return *this;
  }

  ViewIterator& operator++(int) {
    auto cp = *this;
    ++iter_;
    return cp;
  }

  Element operator*() const { return view_->func_(*iter_); }
  Element operator->() const { return view_->func_(iter_.operator->()); }

  bool operator==(const ViewIterator& that) const {
    return iter_ == that.iter_ && view_ == that.view_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter_ != that.iter_ || view_ != that.view_;
  }

  bool is_at_end() { return iter_.is_at_end(); }

 private:
  void move_to_end() { iter_.move_to_end(); }

  const View* view_;
  ViewIterator<PView> iter_;
};

template <typename View, typename PView>
class ViewIterator<
    View, PView,
    FuncType::FILTER> : public std::iterator<std::forward_iterator_tag,
                                             typename View::Element> {
 public:
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view, ViewIterator<PView>(&view->parent_)) {}

  ViewIterator(const View* view, ViewIterator<PView>&& iter)
      : view_(view), iter_(std::move(iter)) {
    move_to_begin();
  }

  ViewIterator& operator++() {
    if (is_at_end()) {
      return *this;
    }

    ++iter_;
    move_while_filtered();

    return *this;
  }

  ViewIterator& operator++(int) {
    auto cp = *this;
    ++*this;
    return cp;
  }

  const Element& operator*() const { return *iter_; }
  const Element& operator->() const { return iter_.operator->(); }

  bool operator==(const ViewIterator& that) const {
    return iter_ == that.iter_ && view_ == that.view_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter_ != that.iter_ || view_ != that.view_;
  }

  bool is_at_end() { return iter_.is_at_end(); }

 private:
  void move_to_end() { iter_.move_to_end(); }

  void move_to_begin() {
    if (is_at_end()) {
      return;
    }

    if (view_->func_(*iter_)) {
      return;
    }
    move_while_filtered();
  }

  void move_while_filtered() {
    while (!is_at_end() && !view_->func_(*iter_)) {
      ++iter_;
    }
  }

  const View* view_;
  ViewIterator<PView> iter_;
};

template <typename View, typename PView>
class ViewIterator<View, PView, FuncType::SKIP> : public std::iterator<
                                                      std::forward_iterator_tag,
                                                      typename View::Element> {
 public:
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view, ViewIterator<PView>(&view->parent_)) {}

  ViewIterator(const View* view, ViewIterator<PView>&& iter)
      : view_(view), iter_(std::move(iter)) {
    move_to_begin();
  }

  ViewIterator& operator++() {
    if (is_at_end()) {
      return *this;
    }

    ++iter_;
    return *this;
  }

  ViewIterator& operator++(int) {
    auto cp = *this;
    ++*this;
    return cp;
  }

  const Element& operator*() const { return *iter_; }
  const Element& operator->() const { return iter_.operator->(); }

  bool operator==(const ViewIterator& that) const {
    return iter_ == that.iter_ && view_ == that.view_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter_ != that.iter_ || view_ != that.view_;
  }

  bool is_at_end() { return iter_.is_at_end(); }

 private:
  void move_to_end() { iter_.move_to_end(); }

  void move_to_begin() { move_while_skipped(); }

  void move_while_skipped() {
    while (!is_at_end() && view_->func_(*iter_)) {
      ++iter_;
    }
  }

  const View* view_;
  ViewIterator<PView> iter_;
};

template <typename View, typename PView>
class ViewIterator<
    View, PView,
    FuncType::FLAT_MAP> : public std::iterator<std::forward_iterator_tag,
                                               typename View::Element> {
 public:
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view, ViewIterator<PView>(&view->parent_)) {}

  ViewIterator(const View* view, ViewIterator<PView>&& iter)
      : view_(view), iter_(std::move(iter)) {
    move_to_begin();
  }

  ViewIterator& operator++() {
    if (is_at_end()) {
      return *this;
    }

    assert(citer_ != cend_ &&
           "Container iterator is at end but the parent iterator is not "
           "not at its end.");

    citer_++;
    if (citer_ == cend_) {
      move_to_nonempty_map();
    }
    return *this;
  }

  ViewIterator& operator++(int) {
    auto cp = *this;
    ++*this;
    return cp;
  }

  const Element& operator*() const { return *citer_; }
  const Element& operator->() const { return citer_.operator->(); }

  bool operator==(const ViewIterator& that) const {
    return iter_ == that.iter_ && view_ == that.view_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter_ != that.iter_ || view_ != that.view_;
  }

  bool is_at_end() { return iter_.is_at_end(); }

 private:
  void move_to_end() { iter_.move_to_end(); }

  void move_to_begin() { move_to_nonempty_map(); }

  void move_to_nonempty_map() {
    do {
      cntr_ = std::move(view_->func_(*iter_));
      citer_ = cntr_.begin();
      cend_ = cntr_.end();
      *iter_++;
    } while(!is_at_end() && citer_ == cend_);
  }

  const View* view_;
  ViewIterator<PView> iter_;

  using PElem = typename PView::Element;
  using Container = decltype(view_->g(*(PElem*) nullptr));
  using CIter = typename Container::const_iterator;

  Container cntr_;
  CIter citer_;
  CIter cend_;
};

template <typename View, typename PView>
class ViewIterator<View, PView, FuncType::KEEP> : public std::iterator<
                                                      std::forward_iterator_tag,
                                                      typename View::Element> {
 public:
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view, ViewIterator<PView>(&view->parent_)) {}

  ViewIterator(const View* view, ViewIterator<PView>&& iter)
      : view_(view), iter_(std::move(iter)) {
    maybe_move_to_end();
  }

  ViewIterator& operator++() {
    maybe_move_to_end();

    if (is_at_end()) {
      return *this;
    }

    ++iter_;
    return *this;
  }

  ViewIterator& operator++(int) {
    auto cp = *this;
    ++*this;
    return cp;
  }

  const Element& operator*() const { return *iter_; }
  const Element& operator->() const { return iter_.operator->(); }

  bool operator==(const ViewIterator& that) const {
    return iter_ == that.iter_ && view_ == that.view_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter_ != that.iter_ || view_ != that.view_;
  }

  bool is_at_end() { return iter_.is_at_end(); }

 private:
  void move_to_end() { iter_.move_to_end(); }

  void maybe_move_to_end() {
    if (is_at_end() || view_->func_(*iter_)) {
      return;
    }

    move_to_end();
  }

  const View* view_;
  ViewIterator<PView> iter_;
};

template <typename View, typename PView1, typename PView2>
class ViewIterator<
    View, std::pair<PView1, PView2>,
    FuncType::ZIP> : public std::iterator<std::forward_iterator_tag,
                                          typename View::Element> {
 public:
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view, ViewIterator<PView1>(&view->parent_.first),
                     ViewIterator<PView2>(&view->parent_.second)) {}

  ViewIterator(const View* view, ViewIterator<PView1>&& iter1,
               ViewIterator<PView2>&& iter2)
      : view_(view), iter1_(std::move(iter1)), iter2_(std::move(iter2)) {}

  ViewIterator& operator++() {
    ++iter1_;
    ++iter2_;
    return *this;
  }

  ViewIterator& operator++(int) {
    auto cp = *this;
    ++iter1_;
    ++iter2_;
    return cp;
  }

  Element operator*() const { return std::make_pair(*iter1_, *iter2_); }
  Element operator->() const {
    return std::make_pair(iter1_.operator->(), iter2_.operator->());
  }

  bool operator==(const ViewIterator& that) const {
    return iter1_ == that.iter1_ && iter2_ == that.iter2_ &&
           view_ == that.view_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter1_ != that.iter1_ || iter2_ != that.iter2_ ||
           view_ != that.view_;
  }

  bool is_at_end() { return iter1_.is_at_end() || iter2_.is_at_end(); }

 private:
  const View* view_;

  using P1View = typename View::PView::first_type;
  using P2View = typename View::PView::second_type;

  ViewIterator<P1View> iter1_;
  ViewIterator<P2View> iter2_;
};

template <typename View>
class ViewIterator<
    View, void*,
    FuncType::FILTER> : public std::iterator<std::forward_iterator_tag,
                                             typename View::Element> {
 public:
  using CIter = typename View::Container::const_iterator;
  using Element = typename View::Element;

  ViewIterator(const View* view)
      : ViewIterator(view->container_->begin(), view->container_->end()) {}
  ViewIterator(const CIter& iter, const CIter& end) : iter_(iter), end_(end) {}

  ViewIterator& operator++() {
    if (is_at_end()) {
      return *this;
    }

    ++iter_;
    return *this;
  }

  ViewIterator operator++(int) {
    auto cp = *this;
    ++iter_;
    return cp;
  }

  const Element& operator*() const { return *iter_; }
  const Element& operator->() const { return iter_.operator->(); }

  bool operator==(const ViewIterator& that) const {
    return iter_ == that.iter_;
  }

  bool operator!=(const ViewIterator& that) const {
    return iter_ != that.iter_;
  }

  bool is_at_end() { return iter_ == end_; }

 private:
  void move_to_end() { iter_ = end_; }

  CIter iter_;
  CIter end_;
};

}  // namespace details
}  // namespace func

#endif  // FUNC_DETAILS_H_

