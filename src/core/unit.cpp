#include "core/unit.h"

void UnitBase::RaiseException(std::uint32_t exc_code, CoreState &state) {
  RaiseException(exc_code, 0, state);
}

void UnitBase::RaiseException(std::uint32_t exc_code,
                              std::uint32_t trap_value, CoreState &state) {
  // TODO: exception
}
