#include "core/core.h"

#include <cassert>

#include "define/define.h"
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
  for (auto &&i : state_.regs) i = 0;
  state_.pc = kResetVector;
}

void Core::NextCycle() {
  // fetch instruction
  auto inst_data = state_.bus.ReadWord(state_.pc);
  state_.pc += 4;
  // select functional unit
  auto inst = reinterpret_cast<Inst *>(&inst_data);
  auto it = units_.find(inst->opcode);
  if (it == units_.end()) {
    // TODO: exception
    return;
  }
  // decode & execute
  switch (inst->opcode) {
    // R-type
    case kAMO: case kOp: {
      auto inst_r = reinterpret_cast<InstR *>(&inst_data);
      it->second->ExecuteR(*inst_r, state_);
      break;
    }
    // I-type
    case kLoad: case kMiscMem: case kJALR: case kSystem: {
      auto inst_i = reinterpret_cast<InstI *>(&inst_data);
      it->second->ExecuteI(*inst_i, state_);
      break;
    }
    // S-type
    case kStore: case kBranch: {
      auto inst_s = reinterpret_cast<InstS *>(&inst_data);
      it->second->ExecuteS(*inst_s, state_);
      break;
    }
    // U-type
    case kAUIPC: case kLUI: case kJAL: {
      auto inst_u = reinterpret_cast<InstU *>(&inst_data);
      it->second->ExecuteU(*inst_u, state_);
      break;
    }
    // other
    case kOpImm: {
      auto inst_i = reinterpret_cast<InstI *>(&inst_data);
      switch (inst_i->funct3) {
        case kSLLI: case kSRXI: {
          // treat 'SLLI', 'SRLI' and 'SRAI' as R-type
          auto inst_r = reinterpret_cast<InstR *>(&inst_data);
          it->second->ExecuteR(*inst_r, state_);
          break;
        }
        default: {
          it->second->ExecuteI(*inst_i, state_);
          break;
        }
      }
      break;
    }
    default: assert(false);
  }
  state_.regs[0] = 0;
}