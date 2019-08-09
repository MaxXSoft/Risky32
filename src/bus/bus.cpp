#include "bus/bus.h"

bool Bus::AddPeripheral(std::uint32_t base_addr, std::uint32_t size,
                        const PeripheralPtr &peripheral) {
  // 'size' must be power of 2
  if ((size & -size) != size) return false;
  auto mask = ~(size - 1);
  // address space does not allow overlap
  for (const auto &i : peripherals_) {
    if (i.base_addr >= base_addr && i.base_addr < base_addr + size) {
      return false;
    }
    if (base_addr >= i.base_addr && base_addr <= i.base_addr + ~i.mask) {
      return false;
    }
  }
  // add io device
  peripherals_.push_back({base_addr, mask, peripheral});
  return true;
}

PeripheralInterface *Bus::GetPeripheral(std::uint32_t addr) {
  // TODO: optimize time complexity (<= O(logn))
  for (const auto &i : peripherals_) {
    if ((addr & i.mask) == i.base_addr) {
      return i.peripheral.get();
    }
  }
  return nullptr;
}

PeripheralInterface *Bus::GetPeripheral(std::uint32_t addr,
                                        std::uint32_t &offset) {
  // TODO: optimize time complexity (<= O(logn))
  for (const auto &i : peripherals_) {
    if ((addr & i.mask) == i.base_addr) {
      offset = addr & ~i.mask;
      return i.peripheral.get();
    }
  }
  return nullptr;
}

std::uint8_t Bus::ReadByte(std::uint32_t addr) {
  std::uint32_t offset;
  auto io = GetPeripheral(addr, offset);
  return io ? io->ReadByte(offset) : 0;
}

void Bus::WriteByte(std::uint32_t addr, std::uint8_t value) {
  std::uint32_t offset;
  auto io = GetPeripheral(addr, offset);
  if (io) io->WriteByte(offset, value);
}

std::uint16_t Bus::ReadHalf(std::uint32_t addr) {
  std::uint32_t offset;
  auto io = GetPeripheral(addr, offset);
  return io ? io->ReadHalf(offset) : 0;
}

void Bus::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  std::uint32_t offset;
  auto io = GetPeripheral(addr, offset);
  if (io) io->WriteHalf(offset, value);
}

std::uint32_t Bus::ReadWord(std::uint32_t addr) {
  std::uint32_t offset;
  auto io = GetPeripheral(addr, offset);
  return io ? io->ReadWord(offset) : 0;
}

void Bus::WriteWord(std::uint32_t addr, std::uint32_t value) {
  std::uint32_t offset;
  auto io = GetPeripheral(addr, offset);
  if (io) io->WriteWord(offset, value);
}
