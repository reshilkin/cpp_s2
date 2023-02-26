#pragma once
#include <cassert>
#include <iterator>

template <typename T>
struct list {
private:
  struct node_base {
    node_base* prev{nullptr};
    node_base* next{nullptr};

    node_base() {
      prev = this;
      next = this;
    }
  };

  struct node : node_base {
    node(T const& e) : val(e) {}
    T val;
  };
  template <typename S>
  struct Iterator {

    using value_type = S;

    using reference = S& ;

    using pointer = S *;

    using iterator_category = std::bidirectional_iterator_tag;

    using difference_type = ptrdiff_t;

    Iterator() = default;

  private:

    Iterator(node_base* ptr) : ptr_(ptr) {} // private + friend list

  public:

    Iterator(Iterator const& other) : ptr_(other.ptr_) {}

    template <typename A,
              std::enable_if_t<std::is_same<S, const A>::value, bool> = true>
    Iterator(Iterator<A> const& other) : ptr_(other.ptr_) {}

    reference operator*() const {
      return to_node(ptr_)->val;
    }

    pointer operator->() const {
      return &(to_node(ptr_)->val);
    }

    Iterator& operator++() {
      ptr_ = ptr_->next;
      return *this;
    }

    Iterator& operator--() {
      ptr_ = ptr_->prev;
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    Iterator operator--(int) {
      Iterator tmp = *this;
      --(*this);
      return tmp;
    }

    friend bool operator==(Iterator const& a, Iterator const& b) {
      return a.ptr_ == b.ptr_;
    }

    friend bool operator!=(Iterator const& a, Iterator const& b) {
      return a.ptr_ != b.ptr_;
    }

    friend list;

  private:
    node_base* ptr_{nullptr};
  };

public:
  // bidirectional iterator
  using iterator = Iterator<T>;
  // bidirectional iterator
  using const_iterator = Iterator<const T>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // O(1)
  list() noexcept = default;

  // O(n), strong
  list(list const& other) : list() {
    for (const auto& x : other) {
      push_back(x);
    }
  }

  // O(n), strong
  list& operator=(list const& other) {
    if (&other == this) {
      return *this;
    }
    list tmp(other);
    swap(tmp, *this);
    return *this;
  }

  // O(n)
  ~list() {
    clear();
  }

  // O(1)
  bool empty() const noexcept {
    return fake.next == &fake;
  }

  // O(1)
  T& front() noexcept {
    return *(begin());
  }

  // O(1)
  T const& front() const noexcept {
    return *(begin());
  }

  // O(1), strong
  void push_front(T const& element) {
    insert(begin(), element);
  }

  // O(1)
  void pop_front() noexcept {
    erase(begin());
  }

  // O(1)
  T& back() noexcept {
    return *(--end());
  }

  // O(1)
  T const& back() const noexcept {
    return *(--end());
  }

  // O(1), strong
  void push_back(T const& element) {
    insert(end(), element);
  }

  // O(1)
  void pop_back() noexcept {
    erase(--end());
  }

  // O(1)
  iterator begin() noexcept {
    return iterator(fake.next);
  }

  // O(1)
  const_iterator begin() const noexcept {
    return iterator(fake.next);
  }

  // O(1)
  iterator end() noexcept {
    return iterator(&fake);
  }

  // O(1)
  const_iterator end() const noexcept {
    return const_iterator(const_cast<node_base*>(&fake));
  }

  // O(1)
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  // O(1)
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  // O(1)
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  // O(1)
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  // O(n)
  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1), strong
  iterator insert(const_iterator pos, T const& val) {
    node* tmp = new node(val);
    node* cur = to_node(pos.ptr_);
    tmp->next = cur;
    tmp->prev = cur->prev;
    cur->prev->next = tmp;
    cur->prev = tmp;
    return iterator(tmp);
  }

  // O(1)
  iterator erase(const_iterator pos) noexcept {
    return erase(pos, std::next(pos, 1));
  }

  // O(n)
  iterator erase(const_iterator first, const_iterator last) noexcept {
    node_base* end = last.ptr_;
    node_base* beg = first.ptr_->prev;
    for(node_base* it = first.ptr_; it != last.ptr_;) {
      node_base* tmp = it->next;
      delete(to_node(it));
      it = tmp;
    }
    link(beg, end);
    return iterator(end);
  }

  // O(1)
  void splice(const_iterator pos, list& other, const_iterator first,
              const_iterator last) noexcept {
    if(first == last) {
      return;
    }
    auto beg = first.ptr_;
    auto end = last.ptr_->prev;
    link(beg->prev, last.ptr_);
    link(pos.ptr_->prev, beg);
    link(end, pos.ptr_);
  }

  friend void swap(list& a, list& b) noexcept {
    node_base* prev_a = a.fake.prev;
    node_base* prev_b = b.fake.prev;
    node_base* next_a = a.fake.next;
    node_base* next_b = b.fake.next;
    if (!a.empty() && !b.empty()) {
      link(prev_a, &(b.fake));
      link(&(b.fake), next_a);
      link(prev_b, &(a.fake));
      link(&(a.fake), next_b);
    } else if (!a.empty() && b.empty()) {
      link(prev_a, &(b.fake));
      link(&(b.fake), next_a);
      link(&(a.fake), &(a.fake));
    } else if (a.empty() && !b.empty()) {
      swap(b, a);
    }
  }

private:
  static node* to_node(node_base* n) {
    return static_cast<node*>(n);
  }

  static void link(node_base* a, node_base* b) {
    a->next = b;
    b->prev = a;
  }

private:
  node_base fake;
};
