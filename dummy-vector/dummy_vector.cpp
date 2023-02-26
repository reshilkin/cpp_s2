#include "dummy_vector.h"
#include "element.h"
#include <cstddef>

dummy_vector::dummy_vector() : data_(nullptr), size_(0), capacity_(0) {}

dummy_vector::dummy_vector(dummy_vector const& other) {
  size_ = other.size_;
  capacity_ = size_;
  data_ = copy_data(other);
}

dummy_vector& dummy_vector::operator=(dummy_vector const& other) {
  dummy_vector(other).swap(*this);
  return *this;
}

dummy_vector::~dummy_vector() {
  free();
}

dummy_vector::T& dummy_vector::operator[](size_t i) {
  return data_[i];
}

dummy_vector::T const& dummy_vector::operator[](size_t i) const {
  return data_[i];
}

dummy_vector::T* dummy_vector::data() {
  return data_;
}

dummy_vector::T const* dummy_vector::data() const {
  return data_;
}

size_t dummy_vector::size() const {
  return size_;
}

dummy_vector::T& dummy_vector::front() {
  return data_[0];
}

dummy_vector::T const& dummy_vector::front() const {
  return data_[0];
}

dummy_vector::T& dummy_vector::back() {
  return data_[size_ - 1];
}

dummy_vector::T const& dummy_vector::back() const {
  return data_[size_ - 1];
}

void dummy_vector::push_back(T const& element) {
  if (size_ == capacity_) {
    size_t new_capacity = 2 * size_ + 1;
    T* temp = copy_data(*this, new_capacity);
    new (temp + size_) T(element);
    free();
    data_ = temp;
    capacity_ = new_capacity;
    size_++;
  } else {
    new (data_ + size_) T(element);
    size_++;
  }
}

void dummy_vector::pop_back() {
  data_[--size_].~T();
}

bool dummy_vector::empty() const {
  return size_ == 0;
}

size_t dummy_vector::capacity() const {
  return capacity_;
}

void dummy_vector::reserve(size_t n) {
  if (capacity_ >= n) {
    return;
  }
  T* temp = copy_data(*this, n);
  free();
  data_ = temp;
  capacity_ = n;
}

void dummy_vector::shrink_to_fit() {
  if (size_ < capacity_) {
    dummy_vector(*this).swap(*this); // new_size == new_capacity == size_
  }
}

void dummy_vector::clear() {
  for (size_t i = 0; i < size_; i++) {
    pop_back();
  }
}

void dummy_vector::swap(dummy_vector& other) {
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

dummy_vector::iterator dummy_vector::begin() {
  return data_;
}

dummy_vector::iterator dummy_vector::end() {
  return data_ + size_;
}

dummy_vector::const_iterator dummy_vector::begin() const {
  return data_;
}

dummy_vector::const_iterator dummy_vector::end() const {
  return data_ + size_;
}

dummy_vector::iterator dummy_vector::insert(const_iterator pos,
                                            T const& element) {
  size_t position = pos - begin();
  push_back(element);
  for (size_t i = size_ - 1; i > position; i--) {
    std::swap(data_[i], data_[i - 1]);
  }
  return begin() + position;
}

dummy_vector::iterator dummy_vector::erase(const_iterator pos) {
  return erase(pos, pos + 1);
}

dummy_vector::iterator dummy_vector::erase(const_iterator first,
                                           const_iterator last) {
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

void dummy_vector::free() {
  reset();
  operator delete(data_);
  data_ = nullptr;
}

void dummy_vector::reset() {
  for (size_t i = 0; i < size_; i++) {
    data_[i].~T();
  }
}

dummy_vector::T* dummy_vector::copy_data(const dummy_vector& other) {
  return copy_data(other, other.size_);
}

dummy_vector::T* dummy_vector::copy_data(const dummy_vector& other,
                                         size_t new_length) {
  if (new_length == 0) {
    return nullptr;
  }
  T* res = static_cast<T*>(operator new(new_length * sizeof(T)));
  for (size_t i = 0; i < other.size_; i++) {
    new (res + i) T(other[i]);
  }
  return res;
}
