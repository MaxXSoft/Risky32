#ifndef RISKY32_BUS_BUS_H_
#define RISKY32_BUS_BUS_H_

#include <vector>
#include <cstdint>

#include "peripheral/peripheral.h"

class Bus : public PeripheralInterface {
 public:
  Bus() {}

  // add new peripheral to specific address space on the bus
  bool AddPeripheral(std::uint32_t base_addr, std::uint32_t size,
                     const PeripheralPtr &peripheral);
  // get peripheral from specific address
  PeripheralInterface *GetPeripheral(std::uint32_t addr);
  // get peripheral from specific address
  // and return offset address relative to peripheral base address
  PeripheralInterface *GetPeripheral(std::uint32_t addr,
                                     std::uint32_t &offset);

  // read a byte (8-bit) from bus
  std::uint8_t ReadByte(std::uint32_t addr) override;
  // write a byte (8-bit) to bus
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  // read a half word (16-bit) from bus
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  // write a half word (16-bit) to bus
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  // read a word (32-bit) from bus
  std::uint32_t ReadWord(std::uint32_t addr) override;
  // write a word (32-bit) to bus
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;

  std::uint32_t size() const override { return 0; }

 private:
  struct PeripheralItem {
    std::uint32_t base_addr, mask;
    PeripheralPtr peripheral;
  };

  // all of peripherals
  std::vector<PeripheralItem> peripherals_;
};

#endif  // RISKY32_BUS_BUS_H_
