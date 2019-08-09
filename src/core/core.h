#ifndef RISKY32_CORE_CORE_H_
#define RISKY32_CORE_CORE_H_

#include <cstdint>
#include <cstddef>

#include "bus/bus.h"
#include "core/state.h"

class Core {
 public:
  Core(Bus &bus) : state_(bus) {}

  // reset the state of current core
  void Reset();
  // run a cycle
  void NextCycle();

  // getters
  // value of specific register
  std::uint32_t reg(std::size_t addr) { state_.regs[addr]; }
  // value of program counter
  std::uint32_t pc() { return state_.pc; }

 private:
  CoreState state_;
};

#endif  // RISKY32_CORE_CORE_H_
