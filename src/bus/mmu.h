#ifndef RISKY32_BUS_MMU_H_
#define RISKY32_BUS_MMU_H_

#include <cstdint>

#include "peripheral/peripheral.h"
#include "core/control/csr.h"
#include "define/vm.h"

class MMU : public PeripheralInterface {
 public:
  MMU(CSR &csr, const PeripheralPtr &bus)
      : csr_(csr), bus_(bus), is_invalid_(false) {}

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;

  // read instruction (execute from memory)
  std::uint32_t ReadInst(std::uint32_t addr);

  // setters
  void set_is_invalid(bool is_invalid) { is_invalid_ = is_invalid; }

  // getters
  // check if last operation is invalid
  bool is_invalid() const { return is_invalid_; }

 private:
  std::uint32_t GetPhysicalAddr(std::uint32_t addr, bool is_store,
                                bool is_execute);
  bool CheckPTEProperty(const Sv32PTE &pte, bool is_store,
                        bool is_execute);

  CSR &csr_;
  PeripheralPtr bus_;
  bool is_invalid_;
};

#endif  // RISKY32_BUS_MMU_H_
