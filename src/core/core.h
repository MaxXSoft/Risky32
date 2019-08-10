#ifndef RISKY32_CORE_CORE_H_
#define RISKY32_CORE_CORE_H_

#include <unordered_map>
#include <cstdint>
#include <cstddef>

#include "bus/bus.h"
#include "core/state.h"
#include "core/unit.h"

class Core {
 public:
  Core(Bus &bus) : state_(bus) { InitUnits(); }

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
  // initialize all functional units
  void InitUnits();

  // internal state
  CoreState state_;
  // functional units
  std::unordered_map<std::uint32_t, UnitPtr> units_;
};

#endif  // RISKY32_CORE_CORE_H_
