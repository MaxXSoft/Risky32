#include "core/control/csr.h"

namespace {

// get privilege level by address of CSR
inline int GetPrivByCSRAddr(std::uint32_t addr) {
  return (addr >> 8) & 0b11;
}

}  // namespace

void CSR::InitCSR() {
  // CSR that hardwired to zero
  zero_ = 0;
  // supervisor mode CSRs
  sstatus_ = 0;
  sscratch_ = 0;
  sepc_ = 0;
  satp_ = 0;
  // machine mode CSRs
  mstatus_ = 0;
  misa_ = 0x40141101;   // RV32IMA, S-mode & U-mode
  mie_ = 0;
  mtvec_ = 0;
  mscratch_ = 0;
  mepc_ = 0;
  mcause_ = 0;
  mtval_ = 0;
  mip_ = 0;
  // machine mode counters (64-bit)
  mcycle_ = 0;
  minstret_ = 0;
}

void CSR::InitMapping() {
  // user mode CSRs
  csrs_[kCSRCycle]      = IntPtrCast<32>(&mcycle_);
  csrs_[kCSRInstRet]    = IntPtrCast<32>(&minstret_);
  csrs_[kCSRCycleH]     = IntPtrCast<32>(&mcycle_) + 1;
  csrs_[kCSRInstRetH]   = IntPtrCast<32>(&minstret_) + 1;
  // supervisor mode CSRs
  csrs_[kCSRSStatus]    = &sstatus_;
  csrs_[kCSRSIE]        = &zero_;
  csrs_[kCSRSTVec]      = &zero_;
  csrs_[kCSRSCounterEn] = &zero_;
  csrs_[kCSRSScratch]   = &sscratch_;
  csrs_[kCSRSEPC]       = &sepc_;
  csrs_[kCSRSCause]     = &zero_;
  csrs_[kCSRSTVal]      = &zero_;
  csrs_[kCSRSIP]        = &zero_;
  csrs_[kCSRSATP]       = &satp_;
  // machine mode CSRs
  csrs_[kCSRMVenderId]  = &zero_;
  csrs_[kCSRMArchId]    = &zero_;
  csrs_[kCSRMImpId]     = &zero_;
  csrs_[kCSRMHartId]    = &zero_;
  csrs_[kCSRMStatus]    = &mstatus_;
  csrs_[kCSRMISA]       = &misa_;
  csrs_[kCSRMIE]        = &mie_;
  csrs_[kCSRMTVec]      = &mtvec_;
  csrs_[kCSRMCounterEn] = &zero_;
  csrs_[kCSRMScratch]   = &mscratch_;
  csrs_[kCSRMEPC]       = &mepc_;
  csrs_[kCSRMCause]     = &mcause_;
  csrs_[kCSRMTVal]      = &mtval_;
  csrs_[kCSRMIP]        = &mip_;
  csrs_[kCSRPMPCfg0]    = &zero_;
  csrs_[kCSRPMPCfg1]    = &zero_;
  csrs_[kCSRPMPCfg2]    = &zero_;
  csrs_[kCSRPMPCfg3]    = &zero_;
  csrs_[kCSRPMPAddr0]   = &zero_;
  csrs_[kCSRPMPAddr1]   = &zero_;
  csrs_[kCSRPMPAddr2]   = &zero_;
  csrs_[kCSRPMPAddr3]   = &zero_;
  csrs_[kCSRPMPAddr4]   = &zero_;
  csrs_[kCSRPMPAddr5]   = &zero_;
  csrs_[kCSRPMPAddr6]   = &zero_;
  csrs_[kCSRPMPAddr7]   = &zero_;
  csrs_[kCSRPMPAddr8]   = &zero_;
  csrs_[kCSRPMPAddr9]   = &zero_;
  csrs_[kCSRPMPAddr10]  = &zero_;
  csrs_[kCSRPMPAddr11]  = &zero_;
  csrs_[kCSRPMPAddr12]  = &zero_;
  csrs_[kCSRPMPAddr13]  = &zero_;
  csrs_[kCSRPMPAddr14]  = &zero_;
  csrs_[kCSRPMPAddr15]  = &zero_;
  csrs_[kMCycle]        = IntPtrCast<32>(&mcycle_);
  csrs_[kMInstRet]      = IntPtrCast<32>(&minstret_);
  csrs_[kMCycleH]       = IntPtrCast<32>(&mcycle_) + 1;
  csrs_[kMInstRetH]     = IntPtrCast<32>(&minstret_) + 1;
  csrs_[kMCountInhibit] = &zero_;
}

CSR::CSR() {
  // current privilege level is M
  cur_priv_ = kPrivLevelM;
  // initialize CSR & mapping
  InitCSR();
  InitMapping();
}

void CSR::UpdateCounter() {
  ++mcycle_;
  ++minstret_;
}

bool CSR::ReadData(std::uint32_t addr, std::uint32_t &value) {
  auto it = csrs_.find(addr);
  if (it == csrs_.end()) {
    // CSR does not exist, illegal instruction
    return false;
  }
  else {
    // check if accessing CSRs in upper privilege level
    if (cur_priv_ < GetPrivByCSRAddr(addr)) return false;
    switch (addr) {
      case kCSRTime: {
        // TODO: read from CLINT
        break;
      }
      case kCSRTimeH: {
        // TODO: read from CLINT
        break;
      }
      default: {
        // return value
        value = *it->second;
        break;
      }
    }
    return true;
  }
}

bool CSR::WriteData(std::uint32_t addr, std::uint32_t value) {
  auto it = csrs_.find(addr);
  if (it == csrs_.end()) {
    // CSR does not exist, illegal instruction
    return false;
  }
  else {
    // check if accessing CSRs in upper privilege level
    if (cur_priv_ < GetPrivByCSRAddr(addr)) return false;
    switch (addr) {
      case kCSRSStatus: {
        *it->second = value & kMaskSStatus;
        // sync 'mstatus'
        mstatus_ = (mstatus_ & ~kMaskSStatus) | (value & kMaskSStatus);
        mstatus_ &= kMaskMStatus;
        break;
      }
      case kCSRSATP: {
        *it->second = value & kMaskSATP;
        break;
      }
      case kCSRMStatus: {
        auto mstatus = PtrCast<MStatus>(&value);
        if (mstatus->mpp == kPrivLevelH) mstatus->mpp = 0;
        *it->second = value & kMaskMStatus;
        // sync 'sstatus'
        sstatus_ = (sstatus_ & ~kMaskMStatus) | (value & kMaskMStatus);
        sstatus_ &= kMaskSStatus;
        break;
      }
      case kCSRMIE: {
        *it->second = value & kMaskMIE;
        break;
      }
      case kCSRMTVec: {
        auto mtvec = PtrCast<MTVec>(&value);
        if (mtvec->mode >= 2) mtvec->mode = 0;
        *it->second = value;
        break;
      }
      case kCSRMIP: {
        *it->second = value & kMaskMIP;
        break;
      }
      case kCSRCycle: case kCSRTime: case kCSRInstRet:
      case kCSRCycleH: case kCSRTimeH: case kCSRInstRetH:
      case kCSRMVenderId: case kCSRMArchId: case kCSRMImpId:
      case kCSRMHartId: case kCSRMISA: {
        // read only, do nothing
        break;
      }
      default: {
        // update value
        *it->second = value;
        break;
      }
    }
    // reset hardwired CSR/bits
    zero_ = 0;
    return true;
  }
}
