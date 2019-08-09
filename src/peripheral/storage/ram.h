#ifndef RISKY32_PERIPHERAL_STORAGE_RAM_H_
#define RISKY32_PERIPHERAL_STORAGE_RAM_H_

#include <vector>
#include <cstddef>

#include "peripheral/peripheral.h"

class RAM : public PeripheralInterface {
 public:
  RAM() { ram_.resize(16384); }
  RAM(std::size_t size) { ram_.resize(size); }

  // reset all bytes in RAM to zero
  void Reset();

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;

  // getters
  // size of RAM
  std::size_t size() const { return ram_.size(); }

  // setters
  // reset the size of the RAM
  void set_size(std::size_t size) { ram_.resize(size); }

 private:
  std::vector<std::uint8_t> ram_;
};

#endif  // RISKY32_PERIPHERAL_STORAGE_RAM_H_
