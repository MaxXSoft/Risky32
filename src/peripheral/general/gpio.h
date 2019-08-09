#ifndef RISKY32_PERIPHERAL_GENERAL_GPIO_H_
#define RISKY32_PERIPHERAL_GENERAL_GPIO_H_

#include "peripheral/peripheral.h"

class GPIO : public PeripheralInterface {
 public:
  GPIO() : halt_(false) {}

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;

  // getters
  bool halt() const { return halt_; }

 private:
  // halt flag
  bool halt_;
};

#endif  // RISKY32_PERIPHERAL_GENERAL_GPIO_H_
