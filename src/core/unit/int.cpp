#include "core/unit/int.h"

#include <cassert>

#include "define/exception.h"
#include "util/cast.h"

namespace {

std::uint32_t PerformIntOp(std::uint32_t opr1, std::uint32_t opr2,
                           std::uint32_t funct3, std::uint32_t funct7) {
  if (funct7 == kRV32M) {
    // signed operands
    std::int32_t opr1s = opr1, opr2s = opr2;
    // multiplication & division
    switch (funct3) {
      case kMUL: return (static_cast<std::int64_t>(opr1s) *
                        static_cast<std::int64_t>(opr2s)) & 0xffffffff;
      case kMULH: return (static_cast<std::int64_t>(opr1s) *
                         static_cast<std::int64_t>(opr2s)) >> 32;
      case kMULHSU: return (static_cast<std::int64_t>(opr1s) *
                           static_cast<std::uint64_t>(opr2)) >> 32;
      case kMULHU: return (static_cast<std::uint64_t>(opr1) *
                          static_cast<std::uint64_t>(opr2)) >> 32;
      case kDIV: {
        if (!opr2) {
          // division by zero
          return 0xffffffff;
        }
        else if (opr1 == 0x80000000 && opr2s == -1) {
          // signed overflow
          return 0x80000000;
        }
        else {
          // normal division
          return opr1s / opr2s;
        }
      }
      case kDIVU: return !opr2 ? 0xffffffff : opr1 / opr2;
      case kREM: {
        if (!opr2) {
          // division by zero
          return opr1;
        }
        else if (opr1 == 0x80000000 && opr2s == -1) {
          // signed overflow
          return 0;
        }
        else {
          // normal division
          return opr1s % opr2s;
        }
      }
      case kREMU: return !opr2 ? opr1 : opr1 % opr2;
      // unreachable situation
      // because 'funct3' only has 3 bits and
      // all 8 cases have been handled before
      default: assert(false);
    }
  }
  else {
    // integer operations
    switch (funct3) {
      case kADDSUB: return funct7 == kRV32I2 ? opr1 - opr2 : opr1 + opr2;
      case kSLL: return opr1 << (opr2 & 0b11111);
      case kSLT: return static_cast<std::int32_t>(opr1) <
                        static_cast<std::int32_t>(opr2);
      case kSLTU: return opr1 < opr2;
      case kXOR: return opr1 ^ opr2;
      case kSRX: {
        if (funct7 == kRV32I2) {
          // 'SRA'
          return static_cast<std::int32_t>(opr1) >> (opr2 & 0b11111);
        }
        else {
          // 'SRL'
          return opr1 >> (opr2 & 0b11111);
        }
      }
      case kOR: return opr1 | opr2;
      case kAND: return opr1 & opr2;
      // same as above
      default: assert(false);
    }
  }
}

}  // namespace

void IntUnit::ExecuteR(const InstR &inst, CoreState &state) {
  // get operands
  std::uint32_t opr1, opr2;
  opr1 = state.regs(inst.rs1);
  if (inst.opcode == kOpImm) {
    // 'SLLI', 'SRLI'
    assert(inst.funct3 == kSLLI || inst.funct3 == kSRXI);
    opr2 = inst.rs2;
  }
  else {
    opr2 = state.regs(inst.rs2);
  }
  // check 'funct7' field
  if (inst.funct7 != kRV32I1 && inst.funct7 != kRV32I2 &&
      inst.funct7 != kRV32M) {
    // invalid 'funct7' field
    state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
  }
  // calculate
  state.regs(inst.rd) = PerformIntOp(opr1, opr2, inst.funct3, inst.funct7);
}

void IntUnit::ExecuteI(const InstI &inst, CoreState &state) {
  assert(inst.funct3 != kSRXI);
  // get operands
  auto opr1 = state.regs(inst.rs1);
  // sign-extended
  auto opr2 = inst.imm & 0x800 ? 0xfffff000 | inst.imm : inst.imm;
  // calculate
  state.regs(inst.rd) = PerformIntOp(opr1, opr2, inst.funct3, kRV32I1);
}

void IntUnit::ExecuteS(const InstS &inst, CoreState &state) {
  assert(false);
}

void IntUnit::ExecuteU(const InstU &inst, CoreState &state) {
  if (inst.opcode == kAUIPC) {
    // 'AUIPC'
    state.regs(inst.rd) = state.pc() + (inst.imm << 12);
  }
  else if (inst.opcode == kLUI) {
    // 'LUI'
    state.regs(inst.rd) = inst.imm << 12;
  }
  else {
    state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
  }
}
