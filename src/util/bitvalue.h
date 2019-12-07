#ifndef RISKY32_UTIL_BITVALUE_H_
#define RISKY32_UTIL_BITVALUE_H_

#include <cstdint>
#include <cstddef>
#include <cassert>

#include "util/cast.h"

// TODO: constexpr

// represents a sequence of bits
// can hold up to N bits
template <std::size_t N>
class BitValue {
 public:
  using ValueType = typename IntPtrType<N>::Type;

  BitValue(ValueType value, std::size_t width)
      : value_(value), width_(width) {
    assert(width_ && width_ <= N);
  }

  // get a single bit from current value
  BitValue<N> Get(std::size_t i) { return Extract(i, i); }

  // extract a subsequence of bits from current value
  BitValue<N> Extract(std::size_t hi, std::size_t lo) {
    assert(hi < width_ && hi >= lo);
    auto width = hi - lo + 1;
    auto mask = (1 << width) - 1;
    return BitValue<N>((value_ >> lo) & mask, width);
  }

  // concatenate with another 'BitValue'
  template <std::size_t M>
  auto Concat(const BitValue<M> &rhs) {
    constexpr auto n = N > M ? N : M;
    return BitValue<n>((value() << rhs.width()) | rhs.value(),
                       width() + rhs.width());
  }

  // 'get' operator
  BitValue<N> operator[](std::size_t i) { return Get(i); }

  // 'extract' operator
  BitValue<N> operator()(std::size_t hi, std::size_t lo) {
    return Extract(hi, lo);
  }

  // 'concat' operator
  template <std::size_t M>
  auto operator|(const BitValue<M> &rhs) {
    return Concat(rhs);
  }

  // getters
  ValueType value() const { return value_ & ((1 << width_) - 1); }
  std::size_t width() const { return width_; }

 private:
  ValueType value_;
  std::size_t width_;
};

using Bit32 = BitValue<32>;

#endif  // RISKY32_UTIL_BITVALUE_H_
