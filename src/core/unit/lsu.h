#ifndef RISKY32_CORE_UNIT_LSU_H_
#define RISKY32_CORE_UNIT_LSU_H_

#include "core/unit.h"

class LoadStoreUnit : public UnitInterface {
 public:
  void ExecuteR(const InstR &inst, CoreState &state) override;
  void ExecuteI(const InstI &inst, CoreState &state) override;
  void ExecuteS(const InstS &inst, CoreState &state) override;
  void ExecuteU(const InstU &inst, CoreState &state) override;
};

#endif  // RISKY32_CORE_UNIT_LSU_H_
