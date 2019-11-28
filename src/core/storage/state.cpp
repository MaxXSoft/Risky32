#include "core/storage/state.h"

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
    // update machine mode status CSR
    auto mstatus_val = core_.csr().mstatus();
    auto mstatus = PtrCast<MStatus>(&mstatus_val);
    mstatus->mpie = mstatus->mie;
    mstatus->mie = 0;
    mstatus->mpp = core_.csr().cur_priv();
    core_.csr().set_cur_priv(kPrivLevelM);
    core_.csr().set_mstatus(*IntPtrCast<32>(&mstatus));
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
    // update machine mode status CSR
    auto mstatus_val = core_.csr().mstatus();
    auto mstatus = PtrCast<MStatus>(&mstatus_val);
    mstatus->mie = mstatus->mpie;
    mstatus->mpie = 1;
    core_.csr().set_cur_priv(mstatus->mpp);
    mstatus->mpp = kPrivLevelU;
    core_.csr().set_mstatus(*IntPtrCast<32>(&mstatus));
  }
  else if (mode == kPrivLevelS) {
    // return from S-mode
    next_pc_ = core_.csr().mepc();
    // read sstatus
    std::uint32_t sstatus_val;
    auto ret = core_.csr().ReadData(kCSRSStatus, sstatus_val);
    assert(ret);
    auto sstatus = PtrCast<SStatus>(&sstatus_val);
    // update sstatus
    sstatus->sie = sstatus->spie;
    sstatus->spie = 1;
    auto priv = sstatus->spp;
    sstatus->spp = kPrivLevelU;
    // apply change
    ret = core_.csr().WriteData(kCSRSStatus, *IntPtrCast<32>(&sstatus));
    assert(ret);
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
