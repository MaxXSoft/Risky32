#include "core/storage/csr.h"

namespace {

//

}  // namespace

void CSR::InitCSR() {
  // TODO
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
  // TODO: deal with privilege! (lower cannot access upper)
  auto it = csrs_.find(addr);
  if (it == csrs_.end()) {
    // CSR does not exist, illegal instruction
    return false;
  }
  else {
    value = *it->second;
    return true;
  }
}

bool CSR::WriteData(std::uint32_t addr, std::uint32_t value) {
  // TODO: deal with privilege! (lower cannot access upper)
  auto it = csrs_.find(addr);
  if (it == csrs_.end()) {
    // CSR does not exist, illegal instruction
    return false;
  }
  else {
    // TODO: with bit mask
    // TODO: handle side effects
    // TODO: write to epc and then misalligned?
    // TODO: sync mstatus & sstatus
    // reset hardwired CSR/bits
    zero_ = 0;
    return true;
  }
}
