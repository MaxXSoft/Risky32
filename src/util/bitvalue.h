#ifndef RISKY32_UTIL_BITVALUE_H_
#define RISKY32_UTIL_BITVALUE_H_

#include <cstdint>
#include <cstddef>
#include <cassert>

// represents a sequence of bits
// can hold up to 'sizeof(std::size_t) * 8' bits
class BitValue {
 public:
  constexpr BitValue(std::size_t value, std::size_t width)
      : value_(value), width_(width) {
    assert(width_ && width_ <= sizeof(std::size_t) * 8);
  }

  // get a single bit from current value
  constexpr BitValue Get(std::size_t i) const { return Extract(i, i); }

  // extract a subsequence of bits from current value
  constexpr BitValue Extract(std::size_t hi, std::size_t lo) const {
    assert(hi < width_ && hi >= lo);
    auto width = hi - lo + 1;
    auto mask = (1 << width) - 1;
    return BitValue((value_ >> lo) & mask, width);
  }

  // concatenate with another 'BitValue'
  constexpr BitValue Concat(const BitValue &rhs) const {
    return BitValue((value() << rhs.width()) | rhs.value(),
                    width() + rhs.width());
  }

  // 'get' operator
  constexpr BitValue operator[](std::size_t i) const { return Get(i); }
  constexpr BitValue operator()(std::size_t i) const { return Get(i); }

  // 'extract' operator
  constexpr BitValue operator()(std::size_t hi, std::size_t lo) const {
    return Extract(hi, lo);
  }

  // 'concat' operator
  constexpr BitValue operator|(const BitValue &rhs) const {
    return Concat(rhs);
  }

  // getters
  constexpr std::size_t value() const {
    return value_ & ((1 << width_) - 1);
  }
  constexpr std::size_t width() const { return width_; }

 private:
  std::size_t value_, width_;
};

#endif  // RISKY32_UTIL_BITVALUE_H_
