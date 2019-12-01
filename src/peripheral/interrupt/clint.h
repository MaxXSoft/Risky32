#ifndef RISKY32_PERIPHERAL_INTERRUPT_CLINT_H_
#define RISKY32_PERIPHERAL_INTERRUPT_CLINT_H_

#include <cstdint>

#include "peripheral/peripheral.h"

// core local interrupt controller
// generates M-mode timer interrupt & software interrupt
class CLINT : public PeripheralInterface {
 public:
  CLINT() : timer_int_(false), soft_int_(false), mtime_(0), mtimecmp_(0) {}

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;

  // update timer register
  void UpdateTimer();

  // getters
  // timer interrupt signal
  const bool *timer_int() const { return &timer_int_; }
  // software interrupt signal
  const bool *soft_int() const { return &soft_int_; }

 private:
  bool timer_int_, soft_int_;
  std::uint64_t mtime_, mtimecmp_;
};

#endif  // RISKY32_PERIPHERAL_INTERRUPT_CLINT_H_
