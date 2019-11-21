#include "core/unit/branch.h"

#include <cassert>

#include "define/exception.h"
#include "util/cast.h"

// helper macro for 'BRANCH' instructions
#define DO_BRANCH(target, state)                          \
  do {                                                    \
    if (target & 0b11) {                                  \
      state.RaiseException(kExcInstAddrMisalign, target); \
    }                                                     \
    else {                                                \
      state.pc() = target;                                \
    }                                                     \
  } while (0)

void BranchUnit::ExecuteR(const InstR &inst, CoreState &state) {
  assert(false);
}

void BranchUnit::ExecuteI(const InstI &inst, CoreState &state) {
  // get target address
  auto offset = inst.imm & 0x800 ? 0xfffff000 | inst.imm : inst.imm;
  auto target = (state.regs(inst.rs1) + offset) & ~0b1;
  if (target & 0b11) {
    state.RaiseException(kExcInstAddrMisalign, target);
  }
  // perform 'JALR'
  state.regs(inst.rd) = state.pc() + 4;
  state.pc() = target;
}

void BranchUnit::ExecuteS(const InstS &inst, CoreState &state) {
  // get offset of branch
  auto ofs0 = (inst.imm5 >> 0) & 0x1;
  auto ofs1 = (inst.imm5 >> 1) & 0xf;
  auto ofs2 = (inst.imm7 >> 0) & 0x3f;
  auto ofs3 = (inst.imm7 >> 6) & 0x1;
  auto offset = (ofs3 << 12) | (ofs2 << 5) | (ofs1 << 1) | (ofs0 << 11);
  offset = offset & (1 << 12) ? 0xffffe000 | offset : offset;
  // get target address
  auto target = state.pc() + offset;
  // get src1 & src2
  const auto &src1 = state.regs(inst.rs1), &src2 = state.regs(inst.rs2);
  std::int32_t src1s = src1, src2s = src2;
  // 'BRANCH' instructions
  switch (inst.funct3) {
    case kBEQ: {
      // branch if equal
      if (src1 == src2) DO_BRANCH(target, state);
      break;
    }
    case kBNE: {
      // branch if unqeual
      if (src1 != src2) DO_BRANCH(target, state);
      break;
    }
    case kBLT: {
      // branch if less than (signed)
      if (src1s < src2s) DO_BRANCH(target, state);
      break;
    }
    case kBGE: {
      // branch if greater than or equal (signed)
      if (src1s >= src2s) DO_BRANCH(target, state);
    }
    case kBLTU: {
      // branch if less than (unsigned)
      if (src1 < src2) DO_BRANCH(target, state);
      break;
    }
    case kBGEU: {
      // branch if greater than or equal (unsigned)
      if (src1 >= src2) DO_BRANCH(target, state);
      break;
    }
    default: {
      // invalid 'funct3' field
      state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
      break;
    }
  }
}

void BranchUnit::ExecuteU(const InstU &inst, CoreState &state) {
  // get offset of J-type instruction
  auto ofs0 = (inst.imm >> 0)  & 0xff;
  auto ofs1 = (inst.imm >> 8)  & 0x1;
  auto ofs2 = (inst.imm >> 9)  & 0x3ff;
  auto ofs3 = (inst.imm >> 19) & 0x1;
  auto offset = (ofs3 << 20) | (ofs2 << 1) | (ofs1 << 11) | (ofs0 << 12);
  offset = offset & (1 << 20) ? 0xffe00000 | offset : offset;
  // get target address
  auto target = state.pc() + offset;
  if (target & 0b11) {
    state.RaiseException(kExcInstAddrMisalign, target);
  }
  // perform 'JAL'
  state.regs(inst.rd) = state.pc() + 4;
  state.pc() = target;
}
