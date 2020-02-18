#include "peripheral/general/confreg.h"

#include <cstdio>
#include <cstdlib>

namespace {

constexpr std::uint32_t kAddrExit = 0x0000;
constexpr std::uint32_t kAddrUART = 0x7ff0;

}  // namespace

std::uint8_t ConfReg::ReadByte(std::uint32_t addr) {
  return 0;
}

void ConfReg::WriteByte(std::uint32_t addr, std::uint8_t value) {
  // do nothing
}

std::uint16_t ConfReg::ReadHalf(std::uint32_t addr) {
  return 0;
}

void ConfReg::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  // do nothing
}

std::uint32_t ConfReg::ReadWord(std::uint32_t addr) {
  return 0;
}

void ConfReg::WriteWord(std::uint32_t addr, std::uint32_t value) {
  switch (addr) {
    case kAddrExit: std::exit(0); break;
    case kAddrUART: std::fputc(value, stderr); break;
    default:;
  }
}
