#ifndef RISKY32_CORE_CORE_H_
#define RISKY32_CORE_CORE_H_

#include <unordered_map>
#include <cstdint>
#include <cstddef>

#include "peripheral/peripheral.h"
#include "bus/mmu.h"
#include "core/control/csr.h"
#include "core/storage/state.h"
#include "core/storage/excmon.h"
#include "core/unit.h"

class Core {
 public:
  Core(const PeripheralPtr &bus)
      : timer_int_(nullptr), soft_int_(nullptr), ext_int_(nullptr),
        bus_(bus), mmu_(csr_, bus), state_(*this) {
    InitUnits();
  }

  // reset the state of current core
  void Reset();
  // run a cycle
  void NextCycle();
  // rewind 1 instruction and then execute specific instruction
  // (used by debugger)
  void ReExecute(std::uint32_t inst_data);

  // setters
  void set_timer_int(const bool *timer_int) { timer_int_ = timer_int; }
  void set_soft_int(const bool *soft_int) { soft_int_ = soft_int; }
  void set_ext_int(const bool *ext_int) { ext_int_ = ext_int; }

  // getters
  // timer interrupt
  const bool *timer_int() const { return timer_int_; }
  // software interrupt
  const bool *soft_int() const { return soft_int_; }
  // external interrupt
  const bool *ext_int() const { return ext_int_; }
  // bus
  PeripheralInterface &bus() { return mmu_; }
  // raw bus (without MMU)
  const PeripheralPtr &raw_bus() { return bus_; }
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
  // dispatch and execute
  void Execute(std::uint32_t inst_data, CoreState &state);
  // write back
  void WriteBack(CoreState &state);

  // interrupt signals
  const bool *timer_int_, *soft_int_, *ext_int_;
  // bus
  PeripheralPtr bus_;
  // MMU
  MMU mmu_;
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
