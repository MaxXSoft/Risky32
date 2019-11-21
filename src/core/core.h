#ifndef RISKY32_CORE_CORE_H_
#define RISKY32_CORE_CORE_H_

#include <unordered_map>
#include <cstdint>
#include <cstddef>

#include "bus/bus.h"
#include "core/storage/state.h"
#include "core/storage/csr.h"
#include "core/storage/excmon.h"
#include "core/unit.h"

class Core {
 public:
  Core(Bus &bus) : bus_(bus), state_(*this) { InitUnits(); }

  // reset the state of current core
  void Reset();
  // run a cycle
  void NextCycle();

  // getters
  // bus
  Bus &bus() { return bus_; }
  // control and status registers
  CSR &csr() { return csr_; }
  // exclusive monitor
  ExclusiveMonitor &exc_mon() { return exc_mon_; }
  // value of specific register
  std::uint32_t reg(std::size_t addr) { return state_.regs(addr); }
  // value of program counter
  std::uint32_t pc() { return state_.pc(); }

 private:
  // initialize all functional units
  void InitUnits();

  // bus
  Bus &bus_;
  // CSR
  CSR csr_;
  // exclusive monitor ('LR' & 'SC')
  ExclusiveMonitor exc_mon_;
  // internal state
  CoreState state_;
  // functional units
  std::unordered_map<std::uint32_t, UnitPtr> units_;
};

#endif  // RISKY32_CORE_CORE_H_
