#include "core/core.h"

#include <cassert>

#include "define/exception.h"
#include "define/inst.h"

// functional units
#include "core/unit/int.h"
#include "core/unit/lsu.h"
#include "core/unit/branch.h"
#include "core/unit/system.h"

// helper macro
#define CHECK_PAGE_FAULT(exc_code)               \
  do {                                           \
    if (mmu_.is_invalid()) {                     \
      state.RaiseException(exc_code, inst_data); \
    }                                            \
  } while (0)

void Core::InitUnits() {
  // create units
  auto int_unit     = std::make_shared<IntUnit>();
  auto load_store   = std::make_shared<LoadStoreUnit>();
  auto branch_unit  = std::make_shared<BranchUnit>();
  auto system_unit  = std::make_shared<SystemUnit>();
  // initialize unit map
  units_[kLoad]     = load_store;
  units_[kMiscMem]  = load_store;
  units_[kOpImm]    = int_unit;
  units_[kAUIPC]    = int_unit;
  units_[kStore]    = load_store;
  units_[kAMO]      = load_store;
  units_[kOp]       = int_unit;
  units_[kLUI]      = int_unit;
  units_[kBranch]   = branch_unit;
  units_[kJALR]     = branch_unit;
  units_[kJAL]      = branch_unit;
  units_[kSystem]   = system_unit;
}

void Core::Execute(std::uint32_t inst_data, CoreState &state) {
  // select functional unit
  auto inst = reinterpret_cast<Inst *>(&inst_data);
  auto it = units_.find(inst->opcode);
  if (it == units_.end()) {
    // illegal instruction
    state.RaiseException(kExcIllegalInst, inst_data);
  }
  else {
    // decode & execute
    switch (inst->opcode) {
      // R-type
      case kAMO: case kOp: {
        auto inst_r = reinterpret_cast<InstR *>(&inst_data);
        it->second->ExecuteR(*inst_r, state);
        // check MMU exception
        CHECK_PAGE_FAULT(kExcStAMOPageFault);
        break;
      }
      // I-type
      case kLoad: case kMiscMem: case kJALR: {
        auto inst_i = reinterpret_cast<InstI *>(&inst_data);
        it->second->ExecuteI(*inst_i, state);
        // check MMU exception
        CHECK_PAGE_FAULT(kExcLoadPageFault);
        break;
      }
      // S-type
      case kStore: case kBranch: {
        auto inst_s = reinterpret_cast<InstS *>(&inst_data);
        it->second->ExecuteS(*inst_s, state);
        // check MMU exception
        CHECK_PAGE_FAULT(kExcStAMOPageFault);
        break;
      }
      // U-type
      case kAUIPC: case kLUI: case kJAL: {
        auto inst_u = reinterpret_cast<InstU *>(&inst_data);
        it->second->ExecuteU(*inst_u, state);
        break;
      }
      // other (immediate)
      case kOpImm: {
        auto inst_i = reinterpret_cast<InstI *>(&inst_data);
        switch (inst_i->funct3) {
          case kSLLI: case kSRXI: {
            // treat 'SLLI', 'SRLI' and 'SRAI' as R-type
            auto inst_r = reinterpret_cast<InstR *>(&inst_data);
            it->second->ExecuteR(*inst_r, state);
            break;
          }
          default: {
            it->second->ExecuteI(*inst_i, state);
            break;
          }
        }
        break;
      }
      // other (system)
      case kSystem: {
        auto inst_r = reinterpret_cast<InstR *>(&inst_data);
        if (inst_r->funct7 == kSFENCE) {
          // 'SFENCE.VMA' instruction
          it->second->ExecuteR(*inst_r, state);
        }
        else {
          // other privileged instructions
          auto inst_i = reinterpret_cast<InstI *>(&inst_data);
          it->second->ExecuteI(*inst_i, state);
        }
        break;
      }
      default: {
        // just ignore
        // since 'illegal instruction' exception has been handled before
        assert(false);
        break;
      }
    }
  }
}

void Core::Reset() {
  state_.Reset();
}

void Core::NextCycle() {
  // reset MMU state
  mmu_.set_is_invalid(false);
  // fetch instruction
  auto inst_data = mmu_.ReadInst(state_.pc());
  auto state = state_;
  state.next_pc() = state.pc() + 4;
  // check MMU exception
  if (mmu_.is_invalid()) {
    state.RaiseException(kExcInstPageFault, inst_data);
  }
  else {
    // dispatch and execute
    Execute(inst_data, state);
  }
  // handle interrupt & exception
  if (state.next_pc() & 0b11) {
    state.RaiseException(kExcInstAddrMisalign, state.next_pc());
  }
  else {
    state.CheckInterrupt();
  }
  if (!state.CheckAndClearExcFlag()) {
    // no exception, perform write back operation
    state_ = state;
  }
  // prepare for next cycle
  state_.regs(0) = 0;
  state_.pc() = state.next_pc();
  csr_.UpdateCounter();
}
