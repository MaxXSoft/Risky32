#ifndef RISKY32_PERIPHERAL_PERIPHERAL_H_
#define RISKY32_PERIPHERAL_PERIPHERAL_H_

#include <memory>
#include <cstdint>

class PeripheralInterface {
 public:
  virtual ~PeripheralInterface() = default;
  // read a byte (8-bit) from current peripheral
  virtual std::uint8_t ReadByte(std::uint32_t addr) = 0;
  // write a byte (8-bit) to current peripheral
  virtual void WriteByte(std::uint32_t addr, std::uint8_t value) = 0;
  // read a half word (16-bit) from current peripheral
  virtual std::uint16_t ReadHalf(std::uint32_t addr) = 0;
  // write a half word (16-bit) to current peripheral
  virtual void WriteHalf(std::uint32_t addr, std::uint16_t value) = 0;
  // read a word (32-bit) from current peripheral
  virtual std::uint32_t ReadWord(std::uint32_t addr) = 0;
  // write a word (32-bit) to current peripheral
  virtual void WriteWord(std::uint32_t addr, std::uint32_t value) = 0;
};

using PeripheralPtr = std::shared_ptr<PeripheralInterface>;

#endif  // RISKY32_PERIPHERAL_PERIPHERAL_H_
