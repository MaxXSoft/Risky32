#include "core/storage/state.h"

#include <functional>
#include <cassert>

#include "core/core.h"
#include "define/exception.h"
#include "define/csr.h"
#include "util/cast.h"

namespace {

inline int GetExcPriority(std::uint32_t exc_code) {
  switch (exc_code) {
    case kExcStAMOAccFault: case kExcLoadAccFault: return 1;
    case kExcStAMOPageFault: case kExcLoadPageFault: return 2;
    case kExcStAMOAddrMisalign: case kExcLoadAddrMisalign: return 3;
    case kExcIllegalInst: case kExcInstAddrMisalign:
    case kExcMEnvCall: case kExcBreakpoint: return 4;
    case kExcInstAccFault: return 5;
    case kExcInstPageFault: return 5;
    default: return 0;
  }
}

template <typename T>
std::uint32_t UpdateCSR(CSR &csr, std::uint32_t addr,
                        std::function<std::uint32_t(T &)> callback) {
  // read CSR data
  std::uint32_t value;
  auto ans = csr.ReadData(addr, value);
  assert(ans);
  auto data = PtrCast<T>(&value);
  // update CSR data
  auto ret = callback(*data);
  // apply change
  ans = csr.WriteData(addr, *IntPtrCast<32>(&data));
  assert(ans);
  return ret;
}

}  // namespace

void CoreState::Reset() {
  for (auto &&i : regs_) i = 0;
  pc_ = kResetVector;
}

bool CoreState::CheckAndClearExcFlag() {
  if (exc_code_) {
    exc_code_ = 0;
    // set machine mode EPC & next pc
    core_.csr().set_mepc(pc_);
    next_pc_ = core_.csr().trap_vec();
    // update current privilege level
    auto priv = core_.csr().cur_priv();
    core_.csr().set_cur_priv(kPrivLevelM);
    // update 'mstatus'
    UpdateCSR<MStatus>(core_.csr(), kCSRMStatus, [priv](MStatus &x) {
      x.mpie = x.mie;
      x.mie = 0;
      x.mpp = priv;
      return 0;
    });
    // clear LR/SC flag
    core_.exc_mon().ClearFlag();
    return true;
  }
  else {
    return false;
  }
}

void CoreState::CheckInterrupt() {
  // TODO: check interrupts from CLINT & PLIC, update 'exc_code_'
}

void CoreState::RaiseException(std::uint32_t exc_code) {
  RaiseException(exc_code, 0);
}

void CoreState::RaiseException(std::uint32_t exc_code,
                               std::uint32_t trap_val) {
  // check priority
  if (GetExcPriority(exc_code) > GetExcPriority(exc_code_)) {
    // save exception cause
    exc_code_ = exc_code;
    core_.csr().set_mcause(exc_code);
    core_.csr().set_mtval(trap_val);
  }
}

bool CoreState::ReturnFromTrap(std::uint32_t mode) {
  // check if is illegal instruction
  if (core_.csr().cur_priv() < mode) return false;
  if (mode == kPrivLevelM) {
    // return from M-mode
    next_pc_ = core_.csr().mepc();
    // update 'mstatus'
    auto priv =
        UpdateCSR<MStatus>(core_.csr(), kCSRMStatus, [](MStatus &x) {
          x.mie = x.mpie;
          x.mpie = 1;
          auto priv = x.mpp;
          x.mpp = kPrivLevelU;
          return priv;
        });
    // update current privilege level
    core_.csr().set_cur_priv(priv);
  }
  else if (mode == kPrivLevelS) {
    // return from S-mode
    next_pc_ = core_.csr().mepc();
    // update 'sstatus'
    auto priv =
        UpdateCSR<SStatus>(core_.csr(), kCSRSStatus, [](SStatus &x) {
          x.sie = x.spie;
          x.spie = 1;
          auto priv = x.spp;
          x.spp = kPrivLevelU;
          return priv;
        });
    // update current privilege level
    core_.csr().set_cur_priv(priv);
  }
  else {
    // invalid privilege level
    assert(false);
  }
  // clear LR/SC flag
  core_.exc_mon().ClearFlag();
  return true;
}

Bus &CoreState::bus() { return core_.bus(); }

CSR &CoreState::csr() { return core_.csr(); }

ExclusiveMonitor &CoreState::exc_mon() { return core_.exc_mon(); }
