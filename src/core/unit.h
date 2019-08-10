#ifndef RISKY32_CORE_UNIT_UNIT_H_
#define RISKY32_CORE_UNIT_UNIT_H_

#include <memory>
#include <cstdint>

#include "define/inst.h"
#include "core/state.h"

class UnitBase {
 public:
  virtual ~UnitBase() = default;

  // execute a R-type instruction
  virtual void ExecuteR(const InstR &inst, CoreState &state) = 0;
  // execute a I-type instruction
  virtual void ExecuteI(const InstI &inst, CoreState &state) = 0;
  // execute a S-type instruction
  virtual void ExecuteS(const InstS &inst, CoreState &state) = 0;
  // execute a U-type instruction
  virtual void ExecuteU(const InstU &inst, CoreState &state) = 0;

  // exception handler
  static void RaiseException(std::uint32_t exc_code, CoreState &state);
};

using UnitPtr = std::shared_ptr<UnitBase>;

#endif  // RISKY32_CORE_UNIT_UNIT_H_
