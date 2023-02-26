#include "big_integer.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <ostream>
#include <stdexcept>

static const uint32_t BASE = 32;

big_integer::big_integer() : sign(0) {}

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned int a)
    : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(long int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(long unsigned int a)
    : big_integer(static_cast<unsigned long long>(a)) {}

void big_integer::pushBits(uint64_t a) {
  while (a != 0) {
    num.push_back(a & std::numeric_limits<uint32_t>::max());
    a >>= BASE;
  }
  fixLeadingBits();
}

big_integer::big_integer(long long a) : sign(a >= 0 ? 0 : 1) {
  pushBits(a & std::numeric_limits<uint64_t>::max());
}

big_integer::big_integer(long long unsigned a) : sign(0) {
  pushBits(a & std::numeric_limits<uint64_t>::max());
}

big_integer::big_integer(std::string const& str) : sign(0) {
  num.push_back(0);
  if (str.size() == 0 || (str[0] == '-' && str.size() == 1)) {
    throw std::invalid_argument("Got empty string in number constructor");
  }
  for (size_t i = str[0] == '-' ? 1 : 0; i < str.size(); i++) {
    if (str[i] > '9' || str[i] < '0') {
      throw std::invalid_argument("Wrong number format");
    }
  }
  for (size_t i = str[0] == '-' ? 1 : 0; i < str.size(); i += 9) {
    uint32_t cur = 0;
    uint32_t factor = 1;
    for(size_t j = 0; j < std::min(str.size() - i, 9ul); j++, factor *= 10) {
      cur *= 10;
      cur += str[i + j] - '0';
    }
    mulShort(factor);
    addShort(cur);
  }
  if (str[0] == '-') {
    negate();
  }
  fixLeadingBits();
}

big_integer& big_integer::operator=(big_integer const& other) {
  big_integer(other).swap(*this);
  return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
  uint32_t carry = 0;
  size_t len = std::max(num.size(), rhs.num.size());
  setLen(len + 1);
  for (size_t i = 0; i < num.size(); i++) {
    uint64_t sum = num[i];
    sum += carry;
    sum += (i < rhs.num.size() ? rhs.num[i] : rhs.signBits());
    uint32_t temp = sum % (1ll << BASE);
    num[i] = temp;
    carry = sum >> BASE;
  }
  sign = leadingBit();
  fixLeadingBits();
  return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
  uint64_t carry = 0;
  size_t len = std::max(num.size(), rhs.num.size());
  setLen(len + 1);
  for (size_t i = 0; i < num.size(); i++) {
    uint64_t diff = (1ull << BASE) + num[i];
    diff -= carry + (i < rhs.num.size() ? rhs.num[i] : rhs.signBits());
    num[i] = diff % (1ull << BASE);
    carry = (diff >> BASE) ^ 1;
  }
  sign = leadingBit();
  fixLeadingBits();
  return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
  uint8_t resSign = 0;
  big_integer sec = rhs;
  if (sign != 0) {
    negate();
    resSign ^= 1;
  }
  if (rhs.sign != 0) {
    sec.negate();
    resSign ^= 1;
  }
  big_integer res;
  res.setLen(num.size() + sec.num.size());
  for (size_t i = 0; i < num.size(); i++) {
    uint64_t carry = 0;
    for (size_t j = 0; j < sec.num.size() || carry != 0; j++) {
      uint64_t cur = 1ull * num[i] * (j < sec.num.size() ? sec.num[j] : 0);
      cur += res.num[i + j] + carry;
      res.num[i + j] = cur % (1ull << BASE);
      carry = cur >> BASE;
    }
  }
  res.fixLeadingBits();
  if (resSign != 0) {
    res.negate();
  }
  res.fixLeadingBits();
  *this = res;
  return *this;
}

big_integer& big_integer::addShort(uint32_t rhs) {
  uint64_t carry = rhs;
  setLen(num.size() + 1);
  for (int32_t i = 0; i < num.size(); i++) {
    if (i == num.size()) {
      num.push_back(0);
    }
    uint64_t cur = num[i] + carry;
    num[i] = cur % (1ull << BASE);
    carry = (cur & (1ull << BASE)) >> BASE;
  }
  sign = leadingBit();
  fixLeadingBits();
  return *this;
}

big_integer& big_integer::subShort(uint32_t rhs) {
  uint64_t carry = rhs;
  setLen(num.size() + 1);
  for (uint32_t& i : num) {
    uint64_t diff = (1ull << BASE) + i;
    diff -= carry;
    i = diff % (1ull << BASE);
    carry = (diff >> BASE) ^ 1;
  }
  sign = leadingBit();
  fixLeadingBits();
  return *this;
}

uint32_t big_integer::divRemShort(uint32_t rhs) {
  uint64_t carry = 0;
  for (int32_t i = num.size() - 1; i >= 0; i--) {
    uint64_t cur = num[i] + (carry << BASE);
    num[i] = (cur / rhs) % (1ll << BASE);
    carry = cur % rhs;
  }
  fixLeadingBits();
  return carry;
}

big_integer& big_integer::mulShort(uint32_t rhs) {
  uint64_t carry = 0;
  for (int32_t i = 0; i < num.size() || carry != 0; i++) {
    if (i == num.size()) {
      num.push_back(0);
    }
    uint64_t cur = 1ull * num[i] * rhs + carry;
    num[i] = cur % (1ull << BASE);
    carry = cur >> BASE;
  }
  fixLeadingBits();
  return *this;
}

int32_t big_integer::normalize() {
  uint32_t first = num.back();
  int32_t res = 0;
  if (first == 0) {
    res = BASE - 1;
  } else {
    while (first < (1ull << (BASE - 1))) {
      first <<= 1;
      res++;
    }
  }
  *this <<= res;
  return res;
}

big_integer& big_integer::divRemLong(const big_integer& rhs, bool remNeeded) {
  uint8_t resSign = 0;
  big_integer b = rhs;
  if (*this < 0) {
    negate();
    resSign ^= 1;
  }
  if (b < 0) {
    b.negate();
    resSign ^= 1;
  }
  if (*this < b) {
    if (!remNeeded) {
      *this = 0;
    }
    if ((resSign ^ rhs.sign) != 0) {
      negate();
    }
    return *this;
  }
  int32_t shift = b.normalize();
  *this <<= shift;
  uint32_t n = b.num.size();
  while (b.num[n - 1] == b.signBits()) {
    n--;
  }
  uint64_t nLast = b.num[n - 1];
  uint32_t m = num.size() - n;
  big_integer res = 0;
  b <<= BASE * m;
  if (*this >= b) {
    res++;
    *this -= b;
  }
  for (int32_t i = m - 1; i >= 0; i--) {
    b >>= BASE;
    uint64_t cur = ((1ll * num[n + i] << BASE) + num[n + i - 1]) / nLast;
    cur = std::min(cur,
                   static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()));
    *this -= cur * b;
    while (sign != 0) {
      cur--;
      *this += b;
    }
    res <<= BASE;
    res.addShort(cur);
  }
  if (!remNeeded) {
    if (resSign != 0) {
      res.negate();
    }
    res.fixLeadingBits();
    *this = res;
  } else {
    *this >>= shift;
    if ((resSign ^ rhs.sign) != 0) {
      negate();
    }
  }
  return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
  return divRemLong(rhs, false);
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
  return divRemLong(rhs, true);
}

template <typename F>
void big_integer::makeBinaryBitOp(const big_integer& rhs, F func) {
  size_t len = std::max(num.size(), rhs.num.size());
  setLen(len + 1);
  for (size_t i = 0; i < num.size(); i++) {
    num[i] = func(num[i], i < rhs.num.size() ? rhs.num[i] : rhs.signBits());
  }
  sign = leadingBit();
  fixLeadingBits();
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
  makeBinaryBitOp(rhs, [](uint32_t a, uint32_t b) { return a & b; });
  return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
  makeBinaryBitOp(rhs, [](uint32_t a, uint32_t b) { return a | b; });
  return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
  makeBinaryBitOp(rhs, [](uint32_t a, uint32_t b) { return a ^ b; });
  return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
  setLen(num.size() + (rhs + BASE - 1) / BASE);
  int32_t mod = rhs % BASE;
  if (mod != 0) {
    uint64_t carry = 0;
    for (uint32_t& i : num) {
      uint64_t temp = (i & (((1u << mod) - 1) << (BASE - mod))) >> (BASE - mod);
      i = (i << mod) + carry;
      carry = temp;
    }
  }
  for (int32_t i = num.size() - 1; i >= (int32_t)(rhs / BASE); i--) {
    num[i] = num[i - rhs / BASE];
  }
  for (int32_t i = rhs / BASE - 1; i >= 0; i--) {
    num[i] = 0;
  }
  fixLeadingBits();
  return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
  setLen(num.size() + (rhs + BASE - 1) / BASE);
  uint32_t carry = 0;
  int mod = rhs % BASE;
  if (mod != 0) {
    for (int32_t i = num.size() - 1; i >= 0; i--) {
      uint32_t temp = num[i] & ((1u << mod) - 1);
      num[i] = (num[i] >> mod) + (carry << (BASE - mod));
      carry = temp;
    }
  }
  for (size_t i = 0; i < num.size() - (rhs + BASE - 1) / BASE; i++) {
    num[i] = num[i + rhs / BASE];
  }
  for (size_t i = 0; i < (rhs + BASE - 1) / BASE; i++) {
    num.pop_back();
  }
  fixLeadingBits();
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

void big_integer::invert() {
  sign ^= 1;
  for (uint32_t& i : num) {
    i = ~i;
  }
  fixLeadingBits();
}

void big_integer::negate() {
  if (*this != 0) {
    invert();
    addShort(1);
  }
}

big_integer big_integer::operator-() const {
  big_integer res(*this);
  res.negate();
  res.fixLeadingBits();
  return res;
}

big_integer big_integer::operator~() const {
  big_integer res = *this;
  res.invert();
  return res;
}

big_integer& big_integer::operator++() {
  addShort(1);
  return *this;
}

big_integer big_integer::operator++(int) {
  big_integer res = *this;
  ++*this;
  return res;
}

big_integer& big_integer::operator--() {
  subShort(1);
  return *this;
}

big_integer big_integer::operator--(int) {
  big_integer res = *this;
  --*this;
  return res;
}

big_integer operator+(big_integer a, big_integer const& b) {
  a += b;
  return a;
}

big_integer operator-(big_integer a, big_integer const& b) {
  a -= b;
  return a;
}

big_integer operator*(big_integer a, big_integer const& b) {
  a *= b;
  return a;
}

big_integer operator/(big_integer a, big_integer const& b) {
  a /= b;
  return a;
}

big_integer operator%(big_integer a, big_integer const& b) {
  a %= b;
  return a;
}

big_integer operator&(big_integer a, big_integer const& b) {
  a &= b;
  return a;
}

big_integer operator|(big_integer a, big_integer const& b) {
  return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
  a ^= b;
  return a;
}

big_integer operator<<(big_integer a, int b) {
  return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
  return a >>= b;
}

int32_t big_integer::compareTo(const big_integer& a) const {
  if (sign == 0 && a.sign != 0) {
    return 1;
  }
  if (sign != 0 && a.sign == 0) {
    return -1;
  }
  for (int32_t i = std::max(a.num.size(), num.size()) - 1; i >= 0; i--) {
    int64_t cur = (i < num.size() ? num[i] : signBits());
    cur -= (i < a.num.size() ? a.num[i] : a.signBits());
    if (cur > 0) {
      return 1;
    } else if (cur < 0) {
      return -1;
    }
  }
  return 0;
}

bool operator==(big_integer const& a, big_integer const& b) {
  return a.compareTo(b) == 0;
}

bool operator!=(big_integer const& a, big_integer const& b) {
  return a.compareTo(b) != 0;
}

bool operator<(big_integer const& a, big_integer const& b) {
  return a.compareTo(b) < 0;
}

bool operator>(big_integer const& a, big_integer const& b) {
  return a.compareTo(b) > 0;
}

bool operator<=(big_integer const& a, big_integer const& b) {
  return a.compareTo(b) <= 0;
}

bool operator>=(big_integer const& a, big_integer const& b) {
  return a.compareTo(b) >= 0;
}

std::string to_string(big_integer const& a) {
  if (a == 0) {
    return "0";
  }
  std::string res;
  big_integer copy;
  copy = a;
  if (a.sign != 0) {
    copy.negate();
  }
  while (copy != 0) {
    uint32_t rem = copy.divRemShort(1000000000);
    std::string tmp = std::to_string(rem);
    std::reverse(tmp.begin(), tmp.end());
    res += tmp;
    if (copy != 0) {
      for (size_t i = tmp.size(); i < 9; i++) {
        res += '0';
      }
    }
  }
  if (a.sign != 0) {
    res += '-';
  }
  std::reverse(res.begin(), res.end());
  return res;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
  return s << to_string(a);
}

uint32_t big_integer::signBits() const {
  return sign == 0 ? 0 : std::numeric_limits<uint32_t>::max();
}

uint8_t big_integer::leadingBit() {
  return num.empty() ? 0 : (num.back() & (1 << (BASE - 1))) >> (BASE - 1);
}

void big_integer::swap(big_integer& other) {
  std::swap(other.num, num);
  std::swap(other.sign, sign);
}

void big_integer::setLen(size_t len) {
  num.reserve(len);
  while (num.size() < len) {
    num.push_back(signBits());
  }
}

void big_integer::fixLeadingBits() {
  while (num.size() > 1 && num.back() == signBits()) {
    num.pop_back();
  }
  if (leadingBit() != sign) {
    num.push_back(signBits());
  }
}
