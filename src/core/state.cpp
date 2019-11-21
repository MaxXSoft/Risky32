#include "core/state.h"

#include "core/core.h"

Bus &CoreState::bus() { return core_.bus(); }

CSR &CoreState::csr() { return core_.csr(); }

void CoreState::RaiseException(std::uint32_t exc_code) {
  RaiseException(exc_code, 0);
}

void CoreState::RaiseException(std::uint32_t exc_code,
                               std::uint32_t trap_val) {
  // TODO: exception
}
