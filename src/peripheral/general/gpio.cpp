#include "peripheral/general/gpio.h"

#include <cstdio>

namespace {

constexpr int kAddrHaltFlag   = 0x100;
constexpr int kAddrConsoleIO  = 0x104;

}  // namespace

std::uint8_t GPIO::ReadByte(std::uint32_t addr) {
  switch (addr) {
    case kAddrHaltFlag: return halt_;
    case kAddrConsoleIO: return std::getchar();
    default: return 0;
  }
}

void GPIO::WriteByte(std::uint32_t addr, std::uint8_t value) {
  switch (addr) {
    case kAddrHaltFlag: halt_ = value;
    case kAddrConsoleIO: std::putchar(value);
    default:;
  }
}

std::uint16_t GPIO::ReadHalf(std::uint32_t addr) {
  return 0;
}

void GPIO::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  // do nothing
}

std::uint32_t GPIO::ReadWord(std::uint32_t addr) {
  return 0;
}

void GPIO::WriteWord(std::uint32_t addr, std::uint32_t value) {
  // do nothing
}
