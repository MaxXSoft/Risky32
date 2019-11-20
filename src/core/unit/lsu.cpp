#include "core/unit/lsu.h"

#include <cassert>

#include "define/exception.h"
#include "util/cast.h"

namespace {

// calculate effective address via base & 12-bit offset
inline std::uint32_t GetAddr(std::uint32_t base, std::uint32_t offset12) {
  auto offset = offset12 & 0x800 ? 0xfffff000 | offset12 : offset12;
  return base + offset;
}

}  // namespace

void LoadStoreUnit::ExecuteR(const InstR &inst, CoreState &state) {
  // AMO
  // TODO: implementation required
  assert(false);
}

void LoadStoreUnit::ExecuteI(const InstI &inst, CoreState &state) {
  if (inst.opcode == kLoad) {
    // get effective address
    auto addr = GetAddr(state.regs[inst.rs1], inst.imm);
    // 'LOAD' instructions
    switch (inst.funct3) {
      case kLB: {
        // load signed byte
        std::int8_t data = state.bus.ReadByte(addr);
        state.regs[inst.rd] = data;
        break;
      }
      case kLH: {
        // load signed half word
        if (addr & 0b1) {
          // misalligned address
          state.RaiseException(kExcLoadAddrMisalign, addr);
        }
        else {
          std::int16_t data = state.bus.ReadHalf(addr);
          state.regs[inst.rd] = data;
        }
        break;
      }
      case kLW: {
        // load word
        if (addr & 0b11) {
          // misalligned address
          state.RaiseException(kExcLoadAddrMisalign, addr);
        }
        else {
          state.regs[inst.rd] = state.bus.ReadWord(addr);
        }
        break;
      }
      case kLBU: {
        // load unsigned byte
        state.regs[inst.rd] = state.bus.ReadByte(addr);
        break;
      }
      case kLHU: {
        // load unsigned half word
        if (addr & 0b1) {
          // misalligned address
          state.RaiseException(kExcLoadAddrMisalign, addr);
        }
        else {
          state.regs[inst.rd] = state.bus.ReadHalf(addr);
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
  else {
    // 'MISC-MEM' instructions
    switch (inst.funct3) {
      case kFENCE: {
        // do nothing because there is no other hart
        break;
      }
      case kFENCEI: {
        // do nothing because there is no I-cache & fetch pipeline
        break;
      }
      default: {
        // invalid 'funct3' field
        state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
        break;
      }
    }
  }
}

void LoadStoreUnit::ExecuteS(const InstS &inst, CoreState &state) {
  // get effective address
  auto addr = GetAddr(state.regs[inst.rs1], (inst.imm7 << 5) | inst.imm5);
  // perform 'STORE'
  switch (inst.funct3) {
    case kSB: {
      // store byte
      state.bus.WriteByte(addr, state.regs[inst.rs2]);
    }
    case kSH: {
      // store half word
      if (addr & 0b1) {
        // misalligned address
        state.RaiseException(kExcStAMOAddrMisalign, addr);
      }
      else {
        state.bus.WriteHalf(addr, state.regs[inst.rs2]);
      }
      break;
    }
    case kSW: {
      // store word
      if (addr & 0b11) {
        // misalligned address
        state.RaiseException(kExcStAMOAddrMisalign, addr);
      }
      else {
        state.bus.WriteWord(addr, state.regs[inst.rs2]);
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

void LoadStoreUnit::ExecuteU(const InstU &inst, CoreState &state) {
  assert(false);
}
