#ifndef RISKY32_CORE_CORE_H_
#define RISKY32_CORE_CORE_H_

#include <cstdint>
#include <cstddef>

#include "bus/bus.h"

class Core {
 public:
  Core(Bus &bus) : bus_(bus) {}

  // reset the state of current core
  void Reset();
  // run a cycle
  void NextCycle();

  // getters
  // value of specific register
  std::uint32_t reg(std::size_t addr) { regs_[addr]; }
  // value of program counter
  std::uint32_t pc() { return pc_; }

 private:
  // system bus
  Bus &bus_;
  // registers
  std::uint32_t regs_[32];
  // program counter
  std::uint32_t pc_;
};

#endif  // RISKY32_CORE_CORE_H_
