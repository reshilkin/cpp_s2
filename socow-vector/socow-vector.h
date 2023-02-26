#pragma once
#include <algorithm>

template <typename T, size_t SMALL_SIZE>
struct socow_vector {
  using iterator = T*;
  using const_iterator = T const*;

  struct shared {
    size_t owners_{1};
    size_t capacity_;
    T data_[0];
  };

  socow_vector(){};

  socow_vector(socow_vector const& other) : size_(other.size()) {
    if (!other.isStatic_) {
      isStatic_ = false;
      fam_ = other.fam_;
      fam_->owners_++;
    } else {
      copy_to(stat_data_, other.data(), size_);
    }
  }

  socow_vector& operator=(const socow_vector& other) {
    if (&other == this) {
      return *this;
    }
    socow_vector(other).swap(*this);
    return *this;
  };

  ~socow_vector() {
    if (isStatic_) {
      stat_reset();
    } else {
      dyn_free();
    }
  }

  T& operator[](size_t i) {
    return data()[i];
  }

  T const& operator[](size_t i) const {
    return data()[i];
  }

  T* data() {
    if (isStatic_) {
      return stat_data_;
    } else {
      make_unique();
      return fam_->data_;
    }
  }

  T const* data() const {
    return get_data();
  }

  size_t size() const {
    return size_;
  }

  T& front() {
    return (*this)[0];
  }

  T const& front() const {
    return (*this)[0];
  }

  T& back() {
    return (*this)[size_ - 1];
  }

  T const& back() const {
    return (*this)[size_ - 1];
  }

  void push_back(T const& element) {
    if (size_ < capacity()) {
      new (data() + size_) T(element);
    } else {
      size_t new_cap = 2 * size_ + 1;
      shared* temp = create_fam(new_cap);
      try {
        copy_to(temp->data_, get_data(), size_);
      } catch (...) {
        free_fam(temp, 0);
        throw;
      }
      try {
        new (temp->data_ + size_) T(element);
      } catch (...) {
        free_fam(temp, size_);
        throw;
      }
      if (isStatic_) {
        stat_reset();
        isStatic_ = false;
      } else {
        dyn_free();
      }
      fam_ = temp;
    }
    size_++;
  }

  void pop_back() {
    data()[size_ - 1].~T();
    size_--;
  }

  bool empty() const {
    return size() == 0;
  }

  size_t capacity() const {
    if (isStatic_) {
      return SMALL_SIZE;
    } else {
      return fam_->capacity_;
    }
  }

  void reserve(size_t new_capacity) {
    if (new_capacity > capacity() || (!isStatic_ && fam_->owners_ > 1)) {
      change_capacity(std::max(new_capacity, capacity()));
    }
  }

  void shrink_to_fit() {
    if (isStatic_ || size_ == capacity()) {
      return;
    }
    if (size_ <= SMALL_SIZE) {
      shared* temp = fam_;
      try {
        copy_to(stat_data_, temp->data_, size_);
      } catch (...) {
        fam_ = temp;
        throw;
      }
      free_fam(temp, size_);
      isStatic_ = true;
    } else {
      change_capacity(size_);
    }
  }

  void clear() {
    if (isStatic_) {
      stat_reset();
    } else {
      dyn_reset();
    }
    size_ = 0;
  }

  void swap(socow_vector& other) {
    if (isStatic_ && other.isStatic_) {
      swap_stat_stat(*this, other);
    } else if (isStatic_) {
      swap_stat_dyn(*this, other);
    } else if (other.isStatic_) {
      swap_stat_dyn(other, *this);
    } else {
      swap_dyn_dyn(*this, other);
    }
  }

  iterator begin() {
    return data();
  }

  iterator end() {
    return data() + size_;
  }

  const_iterator begin() const {
    return data();
  }

  const_iterator end() const {
    return data() + size_;
  }

  iterator insert(const_iterator pos, T const& element) {
    size_t position = pos - get_data();
    push_back(element);
    for (size_t i = size_ - 1; i > position; i--) {
      std::swap(get_data()[i], get_data()[i - 1]);
    }
    return begin() + position;
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    size_t beg = first - get_data();
    size_t len = last - first;
    make_unique();
    for (size_t i = beg; i < size_ - len; i++) {
      std::swap(get_data()[i], get_data()[i + len]);
    }
    for (size_t i = size_ - len; i < size_;) {
      pop_back();
    }
    return begin() + beg;
  }

private:
  T* get_data() {
    if (isStatic_) {
      return stat_data_;
    } else {
      return fam_->data_;
    }
  }

  T const* get_data() const {
    if (isStatic_) {
      return stat_data_;
    } else {
      return fam_->data_;
    }
  }

  void swap_stat_stat(socow_vector& first, socow_vector& second) {
    if (first.size() > second.size()) {
      swap_stat_stat(second, first);
      return;
    }
    size_t len1 = first.size();
    size_t len2 = second.size();
    for (size_t i = 0; i < len1; i++) {
      std::swap(first[i], second[i]);
    }
    copy_to(first.get_data() + len1, second.get_data() + len1, len2 - len1);
    reset(second.get_data() + len1, len2 - len1);
    swap_fields(first, second);
  }

  void swap_stat_dyn(socow_vector& stat, socow_vector& dyn) {
    shared* temp = dyn.fam_;
    try {
      copy_to(dyn.stat_data_, stat.data(), stat.size());
    } catch (...) {
      dyn.fam_ = temp;
      throw;
    }
    stat.stat_reset(stat.size());
    swap_fields(stat, dyn);
    stat.fam_ = temp;
  }

  void swap_dyn_dyn(socow_vector& first, socow_vector& second) {
    swap_fields(first, second);
    std::swap(first.fam_, second.fam_);
  }

  void swap_fields(socow_vector& first, socow_vector& second) {
    std::swap(first.isStatic_, second.isStatic_);
    std::swap(first.size_, second.size_);
  }

  void reset(T* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
      data[i].~T();
    }
  }

  void stat_reset() {
    stat_reset(size_);
  }

  void stat_reset(size_t len) {
    reset(stat_data_, len);
  }

  void dyn_reset() {
    dyn_reset(size_);
  }

  void dyn_reset(size_t len) {
    if (fam_->owners_ == 1) {
      reset(get_data(), len);
    } else {
      size_t cap = capacity();
      fam_->owners_--;
      fam_ = create_fam(cap);
    }
  }

  void dyn_free() {
    dyn_free(size_);
  }

  void dyn_free(size_t len) {
    free_fam(fam_, len);
  }

  void copy_to(T* dest, T const* src, size_t len) {
    for (size_t i = 0; i < len; i++) {
      try {
        new (dest + i) T(src[i]);
      } catch (...) {
        reset(dest, i);
        throw;
      }
    }
  }

  void make_unique() {
    make_unique(capacity());
  }

  void make_unique(size_t new_capacity) {
    if ((isStatic_ || fam_->owners_ == 1) && capacity() >= new_capacity) {
      return;
    }
    shared* temp = create_fam(new_capacity);
    try {
      copy_to(temp->data_, get_data(), size());
    } catch (...) {
      free_fam(temp, 0);
      throw;
    }
    if (isStatic_) {
      stat_reset(size_);
      isStatic_ = false;
    } else {
      fam_->owners_--;
    }
    fam_ = temp;
  }

  void change_capacity(size_t new_capacity) {
    shared* temp = create_fam(new_capacity);
    copy_to(temp->data_, get_data(), size());
    if (isStatic_) {
      stat_reset();
      isStatic_ = false;
    } else {
      dyn_free();
    }
    fam_ = temp;
  }

  shared* create_fam(size_t capacity) {
    size_t bytes = sizeof(shared) + capacity * sizeof(T);
    void* buffer = operator new(bytes);
    auto* res = new (buffer) shared;
    res->capacity_ = capacity;
    return res;
  }

  void free_fam(shared* fam, size_t len) {
    fam->owners_--;
    if (fam->owners_ == 0) {
      reset(fam->data_, len);
      operator delete(fam);
    }
  }

private:
  bool isStatic_{true};
  size_t size_{0};
  union {
    T stat_data_[SMALL_SIZE];
    shared* fam_;
  };
};
