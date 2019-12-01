#ifndef RISKY32_DEFINE_MMIO_H_
#define RISKY32_DEFINE_MMIO_H_

#include <cstdint>

#include "define/exception.h"

// address of memory mapped IOs
constexpr std::uint32_t kMMIOAddrROM    = kResetVector;
constexpr std::uint32_t kMMIOAddrRAM    = 0x80000000;
constexpr std::uint32_t kMMIOAddrGPIO   = 0x90000000;
constexpr std::uint32_t kMMIOAddrCLINT  = 0x90010000;

#endif  // RISKY32_DEFINE_MMIO_H_
