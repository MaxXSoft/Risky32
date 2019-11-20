#ifndef RISKY32_CORE_STATE_H_
#define RISKY32_CORE_STATE_H_

#include <cstdint>
#include <cstdlib>

#include "bus/bus.h"
#include "core/csr.h"

struct CoreState {
  CoreState(Bus &bus, CSR &csr) : bus(bus), csr(csr) {}

  // copy operator
  CoreState &operator=(const CoreState &rhs) {
    if (&rhs != this) {
      std::memcpy(this, &rhs, sizeof(CoreState));
    }
    return *this;
  }

  // raise an exception
  void RaiseException(std::uint32_t exc_code);
  // raise an exception (with trap value required by some exceptions)
  void RaiseException(std::uint32_t exc_code, std::uint32_t trap_val);

  // system bus
  Bus &bus;
  // CSR
  CSR &csr;
  // registers
  std::uint32_t regs[32];
  // program counter
  std::uint32_t pc;
};

#endif  // RISKY32_CORE_STATE_H_
