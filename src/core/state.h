#ifndef RISKY32_CORE_STATE_H_
#define RISKY32_CORE_STATE_H_

#include <cstdint>
#include <cstdlib>

#include "bus/bus.h"
#include "core/csr.h"

// forward declaration of 'Core'
class Core;

class CoreState {
public:
  CoreState(Core &core) : core_(core) {}

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
  // return from trap in specific mode (U, S or M)
  void ReturnFromTrap(std::uint32_t mode);

  // getters
  // bus
  Bus &bus();
  // CSR
  CSR &csr();
  // registers
  std::uint32_t &regs(std::uint32_t addr) { return regs_[addr]; }
  // program counter
  std::uint32_t &pc() { return pc_; }
  // next program counter
  std::uint32_t &next_pc() { return next_pc_; }

private:
  // reference of core
  Core &core_;
  // registers
  std::uint32_t regs_[32];
  // program counter
  std::uint32_t pc_, next_pc_;
};

#endif  // RISKY32_CORE_STATE_H_
