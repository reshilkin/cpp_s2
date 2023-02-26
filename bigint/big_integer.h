#pragma once

#include "vector.h"
#include <iosfwd>
#include <string>

struct big_integer {
  big_integer();
  big_integer(big_integer const& other) = default;
  big_integer(int a);
  big_integer(unsigned a);
  big_integer(long unsigned a);
  big_integer(long a);
  big_integer(long long a);
  big_integer(long long unsigned a);
  explicit big_integer(std::string const& str);
  ~big_integer() = default;

  big_integer& operator=(big_integer const& other);

  big_integer& operator+=(big_integer const& rhs);
  big_integer& operator-=(big_integer const& rhs);
  big_integer& operator*=(big_integer const& rhs);
  big_integer& operator/=(big_integer const& rhs);
  big_integer& operator%=(big_integer const& rhs);

  big_integer& operator&=(big_integer const& rhs);
  big_integer& operator|=(big_integer const& rhs);
  big_integer& operator^=(big_integer const& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);

private:
  // Дополнение до двух, little-endian, старший бит должен совпадать с sign
  uint8_t sign;
  vector<uint32_t> num;
private:
  big_integer& addShort(uint32_t rhs);
  big_integer& subShort(uint32_t rhs);
  big_integer& mulShort(uint32_t rhs);
  big_integer& divRemLong(big_integer const& rhs, bool remNeeded);
  uint32_t divRemShort(uint32_t rhs);
  int32_t compareTo(big_integer const& other) const;
  int32_t normalize();
  void swap(big_integer& other);
  void invert();
  void negate();

  template<typename F>
  void makeBinaryBitOp(big_integer const& rhs, F func);

  uint32_t signBits() const;
  uint8_t leadingBit();
  void fixLeadingBits();
  void pushBits(uint64_t a);
  void setLen(size_t newLen);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);
