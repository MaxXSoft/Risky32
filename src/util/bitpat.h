#ifndef RISKY32_UTIL_BITPAT_H_
#define RISKY32_UTIL_BITPAT_H_

#include <type_traits>
#include <string_view>
#include <cstdint>
#include <cstddef>
#include <cassert>


// bit pattern
template <typename T>
class BitPat {
 public:
  static_assert(std::is_integral<T>::value);

  constexpr BitPat(T value, T mask) : value_(value), mask_(mask) {}
  constexpr BitPat(T value) : value_(value), mask_(-1) {}
  constexpr BitPat(std::string_view s)
      : value_(GetValue(s)), mask_(GetMask(s)) {}

  constexpr bool operator==(const BitPat<T> &rhs) const {
    auto mask = mask_ & rhs.mask_;
    return (value_ & mask) == (rhs.value_ & mask);
  }

  // getters
  constexpr T value() const { return value_; }
  constexpr T mask() const { return mask_; }

 private:
  // get value from string
  constexpr T GetValue(std::string_view s) const {
    assert(s.size() == sizeof(T) * 8);
    T cur_bit = static_cast<T>(1) << (s.size() - 1), v = 0;
    for (const auto &c : s) {
      if (c == '1') v |= cur_bit;
      cur_bit >>= 1;
    }
    return v;
  }

  // get mask from string
  constexpr T GetMask(std::string_view s) const {
    assert(s.size() == sizeof(T) * 8);
    T cur_bit = static_cast<T>(1) << (s.size() - 1), m = 0;
    for (const auto &c : s) {
      if (c == '0' || c == '1') m |= cur_bit;
      cur_bit >>= 1;
    }
    return m;
  }

  T value_, mask_;
};

// bit pattern for 32-bit data
using BitPat32 = BitPat<std::uint32_t>;


// tool for pattern matching of bits
// TODO

#endif  // RISKY32_UTIL_BITPAT_H_
