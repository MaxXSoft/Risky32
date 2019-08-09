#ifndef RISKY32_PERIPHERAL_STORAGE_ROM_H_
#define RISKY32_PERIPHERAL_STORAGE_ROM_H_

#include <string>
#include <vector>

#include "peripheral/peripheral.h"

class ROM : public PeripheralInterface {
 public:
  ROM() {}

  // load binary file to ROM
  bool LoadBinary(const std::string &file);
  // load hexadecimal byte file to ROM
  bool LoadHex(const std::string &file);

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;

  // getters
  // size of ROM
  std::size_t size() const { return rom_.size(); }

 private:
  std::vector<std::uint8_t> rom_;
};

#endif  // RISKY32_PERIPHERAL_STORAGE_ROM_H_
