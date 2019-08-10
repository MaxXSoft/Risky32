#ifndef RISKY32_CORE_STATE_H_
#define RISKY32_CORE_STATE_H_

#include <cstdint>
#include <cstdlib>

#include "bus/bus.h"

struct CoreState {
  CoreState(Bus &bus) : bus(bus) {}
  // copy operator
  CoreState &operator=(const CoreState &rhs) {
    if (&rhs != this) {
      this->bus = rhs.bus;
      std::memcpy(this->regs, rhs.regs, sizeof(regs));
      this->pc = rhs.pc;
    }
    return *this;
  }

  // system bus
  Bus &bus;
  // registers
  std::uint32_t regs[32];
  // program counter
  std::uint32_t pc;
};

#endif  // RISKY32_CORE_STATE_H_
