#ifndef RISKY32_UTIL_CAST_H_
#define RISKY32_UTIL_CAST_H_

#include <cstddef>
#include <cstdint>

// get int type by specifying bit width
template <std::size_t N> struct IntPtrType { using Type = void; };
template <> struct IntPtrType<8> { using Type = std::uint8_t; };
template <> struct IntPtrType<16> { using Type = std::uint16_t; };
template <> struct IntPtrType<32> { using Type = std::uint32_t; };
template <> struct IntPtrType<64> { using Type = std::uint64_t; };

template <std::size_t N, typename T>
inline typename IntPtrType<N>::Type *IntPtrCast(T *ptr) {
  return reinterpret_cast<typename IntPtrType<N>::Type *>(ptr);
}

template <std::size_t N, typename T>
inline const typename IntPtrType<N>::Type *IntPtrCast(const T *ptr) {
  return reinterpret_cast<const typename IntPtrType<N>::Type *>(ptr);
}

template <typename T, typename U>
inline T *PtrCast(U *ptr) {
  return reinterpret_cast<T *>(ptr);
}

template <typename T, typename U>
inline const T *PtrCast(const U *ptr) {
  return reinterpret_cast<const T *>(ptr);
}

#endif  // RISKY32_UTIL_CAST_H_
