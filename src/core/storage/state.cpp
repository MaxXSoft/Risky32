#include "core/storage/state.h"

#include "core/core.h"

Bus &CoreState::bus() { return core_.bus(); }

CSR &CoreState::csr() { return core_.csr(); }

ExclusiveMonitor &CoreState::exc_mon() { return core_.exc_mon(); }

void CoreState::RaiseException(std::uint32_t exc_code) {
  RaiseException(exc_code, 0);
}

void CoreState::RaiseException(std::uint32_t exc_code,
                               std::uint32_t trap_val) {
  // TODO: exception
}
