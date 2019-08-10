#include "core/unit/int.h"

#include <cassert>

#include "define/exception.h"

namespace {

std::uint32_t PerformIntOp(std::uint32_t opr1, std::uint32_t opr2,
                           std::uint32_t funct3, bool is_type_2) {
  switch (funct3) {
    case kADDSUB: return is_type_2 ? opr1 - opr2 : opr1 + opr2;
    case kSLL: return opr1 << (opr2 & 0b11111);
    case kSLT: return static_cast<std::int32_t>(opr1) <
                      static_cast<std::int32_t>(opr2);
    case kSLTU: return opr1 < opr2;
    case kXOR: return opr1 ^ opr2;
    case kSRX: {
      if (is_type_2) {
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
    default: {
      // unreachable situation
      // because 'funct3' only has 3 bits and
      // all 8 cases have been handled before
      assert(false);
    }
  }
}

}  // namespace

void IntUnit::ExecuteR(const InstR &inst, CoreState &state) {
  // get operands
  std::uint32_t opr1, opr2;
  opr1 = state.regs[inst.rs1];
  if (inst.opcode == kOpImm) {
    // 'SLLI', 'SRLI'
    assert(inst.funct3 == kSRXI);
    opr2 = inst.rs2;
  }
  else {
    opr2 = state.regs[inst.rs2];
  }
  // get instruction type
  bool is_type_2;
  if (inst.funct7 == kRV32I1) {
    is_type_2 = false;
  }
  else if (inst.funct7 == kRV32I2) {
    is_type_2 = true;
  }
  else {
    // invalid 'funct7' field
    RaiseException(kExcIllegalInst, state);
  }
  // calculate
  state.regs[inst.rd] = PerformIntOp(opr1, opr2, inst.funct3, is_type_2);
}

void IntUnit::ExecuteI(const InstI &inst, CoreState &state) {
  assert(inst.funct3 != kSRXI);
  // get operands
  auto opr1 = state.regs[inst.rs1];
  // sign-extended
  auto opr2 = inst.imm & 0x800 ? 0xfffff000 | inst.imm : inst.imm;
  // calculate
  state.regs[inst.rd] = PerformIntOp(opr1, opr2, inst.funct3, false);
}

void IntUnit::ExecuteS(const InstS &inst, CoreState &state) {
  assert(false);
}

void IntUnit::ExecuteU(const InstU &inst, CoreState &state) {
  if (inst.opcode == kAUIPC) {
    // 'AUIPC'
    state.regs[inst.rd] = state.pc + (inst.imm << 12);
  }
  else if (inst.opcode == kLUI) {
    // 'LUI'
    state.regs[inst.rd] = inst.imm << 12;
  }
  else {
    RaiseException(kExcIllegalInst, state);
  }
}
