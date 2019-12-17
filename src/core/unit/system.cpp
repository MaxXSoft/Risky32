#include "core/unit/system.h"

#include <cassert>

#include "define/exception.h"
#include "define/csr.h"
#include "util/cast.h"

namespace {

bool PerformPrivileged(const InstI &inst, CoreState &state) {
  switch (inst.imm) {
    case kECALL: {
      // environment call
      switch (state.csr().cur_priv()) {
        case kPrivLevelU: state.RaiseException(kExcUEnvCall); break;
        case kPrivLevelS: state.RaiseException(kExcSEnvCall); break;
        case kPrivLevelM: state.RaiseException(kExcMEnvCall); break;
        default: assert(false);
      }
      break;
    }
    case kEBREAK: {
      // breakpoint
      state.RaiseException(kExcBreakpoint);
      break;
    }
    case kSRET: {
      // return from trap in supervisor mode
      if (!state.ReturnFromTrap(kPrivLevelS)) return false;
      break;
    }
    case kMRET: {
      // return from trap in machine mode
      if (!state.ReturnFromTrap(kPrivLevelM)) return false;
      break;
    }
    case kWFI: {
      // wait for interrupt
      // just implement 'WFI' as a 'NOP'
      break;
    }
    default: {
      // invalid 'imm' field
      return false;
    }
  }
  return true;
}

bool PerformSystem(const InstI &inst, CoreState &state) {
  // 'SYSTEM' instructions
  switch (inst.funct3) {
    case kPRIV: {
      // privileged instructions
      if (inst.rs1 || inst.rd) {
        return false;
      }
      else {
        return PerformPrivileged(inst, state);
      }
      break;
    }
    case kCSRRW: case kCSRRWI: {
      auto val = inst.funct3 == kCSRRW ? state.regs(inst.rs1) : inst.rs1;
      // atomic read/write
      if (inst.rd) {
        if (!state.csr().ReadData(inst.imm, state.regs(inst.rd))) {
          // invalid CSR read
          return false;
        }
      }
      if (!state.csr().WriteData(inst.imm, val)) return false;
      break;
    }
    case kCSRRS: case kCSRRSI: {
      std::uint32_t val;
      auto mask = inst.funct3 == kCSRRS ? state.regs(inst.rs1) : inst.rs1;
      // atomic read and set bits
      if (!state.csr().ReadData(inst.imm, val)) return false;
      state.regs(inst.rd) = val;
      if (inst.rs1) {
        if (!state.csr().WriteData(inst.imm, val | mask)) return false;
      }
      break;
    }
    case kCSRRC: case kCSRRCI: {
      std::uint32_t val;
      auto mask = inst.funct3 == kCSRRC ? state.regs(inst.rs1) : inst.rs1;
      // atomic read and clear bits
      if (!state.csr().ReadData(inst.imm, val)) return false;
      state.regs(inst.rd) = val;
      if (inst.rs1) {
        if (!state.csr().WriteData(inst.imm, val & ~mask)) return false;
      }
      break;
    }
    default: {
      // invalid 'funct3' field
      return false;
    }
  }
  return true;
}

}  // namespace

void SystemUnit::ExecuteR(const InstR &inst, CoreState &state) {
  // 'SFENCE.VMA' instruction
  // already checked 'funct3' and 'funct7' in 'Core::Execute'
  if (!inst.rd && state.csr().cur_priv() >= kPrivLevelS) {
    // do nothing because there is no TLB
  }
  else {
    // illegal privileged instruction
    state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
  }
}

void SystemUnit::ExecuteI(const InstI &inst, CoreState &state) {
  if (!PerformSystem(inst, state)) {
    // illegal instruction
    state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
  }
}

void SystemUnit::ExecuteS(const InstS &inst, CoreState &state) {
  assert(false);
}

void SystemUnit::ExecuteU(const InstU &inst, CoreState &state) {
  assert(false);
}
