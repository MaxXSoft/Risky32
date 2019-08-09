#include "peripheral/storage/ram.h"

#include <cassert>

void RAM::Reset() {
  for (auto &&i : ram_) i = 0;
}

std::uint8_t RAM::ReadByte(std::uint32_t addr) {
  return ram_[addr];
}

void RAM::WriteByte(std::uint32_t addr, std::uint8_t value) {
  ram_[addr] = value;
}

std::uint16_t RAM::ReadHalf(std::uint32_t addr) {
  assert((addr & 1) == 0);
  std::uint16_t half = ram_[addr] | (ram_[addr + 1] << 8);
  return half;
}

void RAM::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  assert((addr & 1) == 0);
  ram_[addr] = value & 0xff;
  ram_[addr + 1] = value >> 8;
}

std::uint32_t RAM::ReadWord(std::uint32_t addr) {
  assert((addr & 3) == 0);
  std::uint32_t word = ram_[addr] | (ram_[addr + 1] << 8);
  word |= (ram_[addr + 2] << 16) | (ram_[addr + 3] << 24);
  return word;
}

void RAM::WriteWord(std::uint32_t addr, std::uint32_t value) {
  assert((addr & 3) == 0);
  ram_[addr] = value & 0xff;
  ram_[addr + 1] = (value >> 8) & 0xff;
  ram_[addr + 2] = (value >> 16) & 0xff;
  ram_[addr + 3] = value >> 24;
}
