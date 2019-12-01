#include "peripheral/interrupt/clint.h"

namespace {

constexpr std::uint32_t kAddrMTimeLo    = 0x000;
constexpr std::uint32_t kAddrMTimeHi    = 0x004;
constexpr std::uint32_t kAddrMTimeCmpLo = 0x100;
constexpr std::uint32_t kAddrMTimeCmpHi = 0x104;
constexpr std::uint32_t kAddrMSIP       = 0x200;

}  // namespace

std::uint8_t CLINT::ReadByte(std::uint32_t addr) {
  return 0;
}

void CLINT::WriteByte(std::uint32_t addr, std::uint8_t value) {
  // do nothing
}

std::uint16_t CLINT::ReadHalf(std::uint32_t addr) {
  return 0;
}

void CLINT::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  // do nothing
}

std::uint32_t CLINT::ReadWord(std::uint32_t addr) {
  switch (addr) {
    case kAddrMTimeLo: return mtime_ & 0xffffffff;
    case kAddrMTimeHi: return mtime_ >> 32;
    case kAddrMTimeCmpLo: return mtimecmp_ & 0xffffffff;
    case kAddrMTimeCmpHi: return mtimecmp_ >> 32;
    case kAddrMSIP: return soft_int_;
    default: return 0;
  }
}

void CLINT::WriteWord(std::uint32_t addr, std::uint32_t value) {
  switch (addr) {
    case kAddrMTimeLo: {
      mtime_ = (mtime_ & 0xffffffff00000000) | value;
      break;
    }
    case kAddrMTimeHi: {
      mtime_ = (mtime_ & 0xffffffff) |
               (static_cast<std::uint64_t>(value) << 32);
      break;
    }
    case kAddrMTimeCmpLo: {
      mtimecmp_ = (mtimecmp_ & 0xffffffff00000000) | value;
      break;
    }
    case kAddrMTimeCmpHi: {
      mtimecmp_ = (mtimecmp_ & 0xffffffff) |
                  (static_cast<std::uint64_t>(value) << 32);
      break;
    }
    case kAddrMSIP: {
      soft_int_ = value;
      break;
    }
    default:;
  }
}

void CLINT::UpdateTimer() {
  ++mtime_;
  timer_int_ = mtime_ >= mtimecmp_;
}
