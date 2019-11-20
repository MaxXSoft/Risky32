#include "core/unit/system.h"

#include <cassert>

#include "define/exception.h"
#include "util/cast.h"

namespace {

void PerformPrivilleged(const InstI &inst, CoreState &state) {
  switch (inst.imm) {
    case kECALL: {
      // environment call
      state.RaiseException(kExcMEnvCall);
      break;
    }
    case kEBREAK: {
      // breakpoint
      state.RaiseException(kExcBreakpoint);
      break;
    }
    case kMRET: {
      // return from trap
      // TODO: implementation required
      assert(false);
      break;
    }
    case kWFI: {
      // wait for interrupt
      // TODO: implementation required
      assert(false);
      break;
    }
    default: {
      // invalid 'imm' field
      state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
      break;
    }
  }
}

}  // namespace

void SystemUnit::ExecuteR(const InstR &inst, CoreState &state) {
  assert(false);
}

void SystemUnit::ExecuteI(const InstI &inst, CoreState &state) {
  // 'SYSTEM' instructions
  switch (inst.funct3) {
    case kPRIV: {
      // privilleged instructions
      if (inst.rs1 || inst.rd) {
        state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
      }
      else {
        PerformPrivilleged(inst, state);
      }
      break;
    }
    case kCSRRW: case kCSRRWI: {
      auto val = inst.funct3 == kCSRRW ? state.regs[inst.rs1] : inst.rs1;
      // atomic read/write
      if (inst.rd) {
        state.regs[inst.rd] = state.csr.ReadData(inst.imm);
      }
      state.csr.WriteData(inst.imm, val);
      break;
    }
    case kCSRRS: case kCSRRSI: {
      auto mask = inst.funct3 == kCSRRS ? state.regs[inst.rs1] : inst.rs1;
      // atomic read and set bits
      auto val = state.csr.ReadData(inst.imm);
      state.regs[inst.rd] = val;
      if (inst.rs1) {
        state.csr.WriteData(inst.imm, val | mask);
      }
      break;
    }
    case kCSRRC: case kCSRRCI: {
      auto mask = inst.funct3 == kCSRRC ? state.regs[inst.rs1] : inst.rs1;
      // atomic read and clear bits
      auto val = state.csr.ReadData(inst.imm);
      state.regs[inst.rd] = val;
      if (inst.rs1) {
        state.csr.WriteData(inst.imm, val & ~mask);
      }
      break;
    }
    default: {
      // invalid 'funct3' field
      state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
      break;
    }
  }
}

void SystemUnit::ExecuteS(const InstS &inst, CoreState &state) {
  assert(false);
}

void SystemUnit::ExecuteU(const InstU &inst, CoreState &state) {
  assert(false);
}
