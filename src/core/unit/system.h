#ifndef RISKY32_CORE_UNIT_SYSTEM_H_
#define RISKY32_CORE_UNIT_SYSTEM_H_

#include "core/unit.h"

class SystemUnit : public UnitBase {
 public:
  void ExecuteR(const InstR &inst, CoreState &state) override;
  void ExecuteI(const InstI &inst, CoreState &state) override;
  void ExecuteS(const InstS &inst, CoreState &state) override;
  void ExecuteU(const InstU &inst, CoreState &state) override;
};

#endif  // RISKY32_CORE_UNIT_SYSTEM_H_
