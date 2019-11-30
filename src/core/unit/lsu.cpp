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

// check if no address exception (for 'AMO' instructions)
inline bool CheckNoAddrExc(std::uint32_t addr, CoreState &state) {
  if (addr & 0b11) {
    state.RaiseException(kExcStAMOAddrMisalign, addr);
    return false;
  }
  return true;
}

inline std::int32_t Min(std::int32_t lhs, std::int32_t rhs) {
  return lhs < rhs ? lhs : rhs;
}

inline std::int32_t Max(std::int32_t lhs, std::int32_t rhs) {
  return lhs > rhs ? lhs : rhs;
}

inline std::uint32_t MinU(std::uint32_t lhs, std::uint32_t rhs) {
  return lhs < rhs ? lhs : rhs;
}

inline std::uint32_t MaxU(std::uint32_t lhs, std::uint32_t rhs) {
  return lhs > rhs ? lhs : rhs;
}

}  // namespace

void LoadStoreUnit::ExecuteR(const InstR &inst, CoreState &state) {
  // get address
  auto addr = state.regs(inst.rs1);
  // 'AMO' instructions
  // ignore all ordering bits
  switch (inst.funct7 & 0b1111100) {
    case kLR: {
      // check exceptions
      if (inst.rs2) {
        // invalid 'rs2' field
        state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
      }
      else if (CheckNoAddrExc(addr, state)) {
        // set flag & load data
        state.exc_mon().SetFlag(addr);
        state.regs(inst.rd) = state.bus().ReadWord(addr);
      }
      break;
    }
    case kSC: {
      if (CheckNoAddrExc(addr, state)) {
        if (state.exc_mon().CheckFlag(addr)) {
          // success
          state.bus().WriteWord(addr, state.regs(inst.rs2));
          state.regs(inst.rd) = 0;
        }
        else {
          // failure
          state.regs(inst.rd) = 1;
        }
        // clear flag
        state.exc_mon().ClearFlag();
      }
      break;
    }
    case kAMOSWAP: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = state.regs(inst.rs2);
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOADD: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = data + state.regs(inst.rs2);
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOXOR: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = data ^ state.regs(inst.rs2);
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOAND: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = data & state.regs(inst.rs2);
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOOR: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = data | state.regs(inst.rs2);
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOMIN: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = Min(data, state.regs(inst.rs2));
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOMAX: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = Max(data, state.regs(inst.rs2));
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOMINU: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = MinU(data, state.regs(inst.rs2));
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    case kAMOMAXU: {
      if (CheckNoAddrExc(addr, state)) {
        auto data = state.bus().ReadWord(addr);
        state.regs(inst.rd) = data;
        auto result = MaxU(data, state.regs(inst.rs2));
        state.bus().WriteWord(addr, result);
      }
      break;
    }
    default: {
      // illegal 'funct7' (actual 'funct5') field
      state.RaiseException(kExcIllegalInst, *IntPtrCast<32>(&inst));
      break;
    }
  }
}

void LoadStoreUnit::ExecuteI(const InstI &inst, CoreState &state) {
  if (inst.opcode == kLoad) {
    // get effective address
    auto addr = GetAddr(state.regs(inst.rs1), inst.imm);
    // 'LOAD' instructions
    switch (inst.funct3) {
      case kLB: {
        // load signed byte
        std::int8_t data = state.bus().ReadByte(addr);
        state.regs(inst.rd) = data;
        break;
      }
      case kLH: {
        // load signed half word
        if (addr & 0b1) {
          // misaligned address
          state.RaiseException(kExcLoadAddrMisalign, addr);
        }
        else {
          std::int16_t data = state.bus().ReadHalf(addr);
          state.regs(inst.rd) = data;
        }
        break;
      }
      case kLW: {
        // load word
        if (addr & 0b11) {
          // misaligned address
          state.RaiseException(kExcLoadAddrMisalign, addr);
        }
        else {
          state.regs(inst.rd) = state.bus().ReadWord(addr);
        }
        break;
      }
      case kLBU: {
        // load unsigned byte
        state.regs(inst.rd) = state.bus().ReadByte(addr);
        break;
      }
      case kLHU: {
        // load unsigned half word
        if (addr & 0b1) {
          // misaligned address
          state.RaiseException(kExcLoadAddrMisalign, addr);
        }
        else {
          state.regs(inst.rd) = state.bus().ReadHalf(addr);
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
  auto addr = GetAddr(state.regs(inst.rs1), (inst.imm7 << 5) | inst.imm5);
  // perform 'STORE'
  switch (inst.funct3) {
    case kSB: {
      // store byte
      state.bus().WriteByte(addr, state.regs(inst.rs2));
    }
    case kSH: {
      // store half word
      if (addr & 0b1) {
        // misaligned address
        state.RaiseException(kExcStAMOAddrMisalign, addr);
      }
      else {
        state.bus().WriteHalf(addr, state.regs(inst.rs2));
      }
      break;
    }
    case kSW: {
      // store word
      if (addr & 0b11) {
        // misaligned address
        state.RaiseException(kExcStAMOAddrMisalign, addr);
      }
      else {
        state.bus().WriteWord(addr, state.regs(inst.rs2));
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
