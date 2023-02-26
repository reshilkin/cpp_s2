#pragma once
#include <algorithm>
#include <cstddef>

template <typename T>
struct vector {
  using iterator = T*;
  using const_iterator = T const*;

  // O(1) nothrow
  vector() : data_(nullptr), size_(0), capacity_(0) {}

  // O(N) strong
  vector(vector<T> const& other) : data_(nullptr), size_(0), capacity_(0) {
    data_ = copy_data(other);
    capacity_ = other.size_;
    size_ = capacity_;
  }

  // O(N) strong
  vector<T>& operator=(vector<T> const& other) {
    if(&other == this) {
      return *this;
    }
    vector(other).swap(*this);
    return *this;
  }

  bool operator==(vector<T> const& other) const {
    if(other.size_ == size_) {
      for(size_t i = 0; i < size_; i++) {
        if(data_[i] != other[i]) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  // O(N) nothrow
  ~vector() {
    free();
  }

  // O(1) nothrow
  T& operator[](size_t i) {
    return data_[i];
  }

  // O(1) nothrow
  T const& operator[](size_t i) const {
    return data_[i];
  }

  // O(1) nothrow
  T* data() {
    return data_;
  }

  // O(1) nothrow
  T const* data() const {
    return data_;
  }

  // O(1) nothrow
  size_t size() const {
    return size_;
  }

  // O(1) nothrow
  T& front() {
    return data_[0];
  }

  // O(1) nothrow
  T const& front() const {
    return data_[0];
  }

  // O(1) nothrow
  T& back() {
    return data_[size_ - 1];
  }

  // O(1) nothrow
  T const& back() const {
    return data_[size_ - 1];
  }

  // O(1)* strong
  void push_back(T const& element) {
    if (size_ == capacity_) {
      size_t new_capacity = 2 * size_ + 1;
      T* temp = copy_data(*this, new_capacity);
      try {
        new (temp + size_) T(element);
      } catch (...) {
        free(temp, size_);
        throw;
      }
      free();
      data_ = temp;
      capacity_ = new_capacity;
    } else {
      new (data_ + size_) T(element);
    }
    size_++;
  }

  // O(1) nothrow
  void pop_back() {
    data_[--size_].~T();
  }

  // O(1) nothrow
  bool empty() const {
    return size_ == 0;
  }

  // O(1) nothrow
  size_t capacity() const {
    return capacity_;
  }

  // O(N) strong
  void reserve(size_t n) {
    if (capacity_ >= n) {
      return;
    }
    T* temp = copy_data(*this, n);
    free();
    data_ = temp;
    capacity_ = n;
  }

  // O(N) strong
  void shrink_to_fit() {
    if (size_ < capacity_) {
      vector(*this).swap(*this); // new_size == new_capacity == size_
    }
  }

  // O(N) nothrow
  void clear() {
    for (size_t i = 0; i < size_; i++) {
      pop_back();
    }
  }

  // O(1) nothrow
  void swap(vector& other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  // O(1) nothrow
  iterator begin() {
    return data_;
  }

  // O(1) nothrow
  iterator end() {
    return data_ + size_;
  }

  // O(1) nothrow
  const_iterator begin() const {
    return data_;
  }

  // O(1) nothrow
  const_iterator end() const {
    return data_ + size_;
  }

  // O(N) strong
  iterator insert(const_iterator pos, T const& element) {
    size_t position = pos - begin();
    push_back(element);
    for (size_t i = size_ - 1; i > position; i--) {
      std::swap(data_[i], data_[i - 1]);
    }
    return begin() + position;
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator first, const_iterator last) {
    size_t beg = first - begin();
    size_t len = last - first;
    for (size_t i = beg; i < size_ - len; i++) {
      std::swap(data_[i], data_[i + len]);
    }
    for (size_t i = size_ - len; i < size_;) {
      pop_back();
    }
    return begin() + beg;
  }

private:
  T* data_;
  size_t size_;
  size_t capacity_;

private:
  void free() {
    free(data_, size_);
    data_ = nullptr;
  }

  void free(T* data, size_t size) {
    reset(data, size);
    operator delete(data);
  }

  void reset(T* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
      data[i].~T();
    }
  }

  T* copy_data(const vector& other) {
    return copy_data(other, other.size_);
  }

  T* copy_data(const vector& other, size_t new_length) {
    if (new_length == 0) {
      return nullptr;
    }
    T* res = static_cast<T*>(operator new(new_length * sizeof(T)));
    size_t i = 0;
    try {
      for (; i < other.size_; i++) {
        new (res + i) T(other[i]);
      }
    } catch (...) {
      free(res, i);
      throw;
    }
    return res;
  }
};
