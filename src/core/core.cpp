#include "core/core.h"

#include <cassert>

#include "define/exception.h"
#include "define/inst.h"

// functional units
#include "core/unit/int.h"
#include "core/unit/lsu.h"
#include "core/unit/branch.h"
#include "core/unit/system.h"

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

void Core::Reset() {
  state_.Reset();
}

void Core::NextCycle() {
  // fetch instruction
  auto inst_data = state_.bus().ReadWord(state_.pc());
  auto state = state_;
  state.next_pc() = state.pc() + 4;
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
        break;
      }
      // I-type
      case kLoad: case kMiscMem: case kJALR: case kSystem: {
        auto inst_i = reinterpret_cast<InstI *>(&inst_data);
        it->second->ExecuteI(*inst_i, state);
        break;
      }
      // S-type
      case kStore: case kBranch: {
        auto inst_s = reinterpret_cast<InstS *>(&inst_data);
        it->second->ExecuteS(*inst_s, state);
        break;
      }
      // U-type
      case kAUIPC: case kLUI: case kJAL: {
        auto inst_u = reinterpret_cast<InstU *>(&inst_data);
        it->second->ExecuteU(*inst_u, state);
        break;
      }
      // other
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
      default: {
        // just ignore
        // since 'illegal instruction' exception has been handled before
        assert(false);
        break;
      }
    }
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
