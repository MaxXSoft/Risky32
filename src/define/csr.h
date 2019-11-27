#ifndef RISKY32_DEFINE_CSR_H_
#define RISKY32_DEFINE_CSR_H_

#include <cstdint>


// supervisor status register
struct SStatus {
  std::uint32_t uie   : 1;  // zero (no N extension)
  std::uint32_t sie   : 1;  // zero (no S-mode trap)
  std::uint32_t wpri0 : 2;  // zero
  std::uint32_t upie  : 1;  // zero (no N extension)
  std::uint32_t spie  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri1 : 2;  // zero
  std::uint32_t spp   : 1;  // S-mode previous privilege level
  std::uint32_t wpri2 : 4;  // zero
  std::uint32_t fs    : 2;  // zero (no FPU)
  std::uint32_t xs    : 2;  // zero (no custom extension)
  std::uint32_t wpri3 : 1;  // zero
  std::uint32_t sum   : 1;  // zero (reduce complexity)
  std::uint32_t mxr   : 1;  // zero (reduce complexity)
  std::uint32_t wpri4 : 11; // zero
  std::uint32_t sd    : 1;  // zero (&xs || &fs)
};

// supervisor address translation and protection register
struct SATP {
  std::uint32_t ppn   : 22; // physical page number
  std::uint32_t asid  : 9;  // zero (reduce complexity)
  std::uint32_t mode  : 1;  // enable/disable translation
};

// machine status register
struct MStatus {
  std::uint32_t uie   : 1;  // zero (no N extension)
  std::uint32_t sie   : 1;  // zero (no S-mode trap)
  std::uint32_t wpri0 : 1;  // zero
  std::uint32_t mie   : 1;  // M-mode interrupt enable
  std::uint32_t upie  : 1;  // zero (no N extension)
  std::uint32_t spie  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri1 : 1;  // zero
  std::uint32_t mpie  : 1;  // M-mode previous interrupt enable
  std::uint32_t spp   : 1;  // S-mode previous privilege level
  std::uint32_t wpri2 : 2;  // zero
  std::uint32_t mpp   : 2;  // M-mode previous privilege level
  std::uint32_t fs    : 2;  // zero (no FPU)
  std::uint32_t xs    : 2;  // zero (no custom extension)
  std::uint32_t mprv  : 1;  // zero (reduce complexity)
  std::uint32_t sum   : 1;  // zero (reduce complexity)
  std::uint32_t mxr   : 1;  // zero (reduce complexity)
  std::uint32_t tvm   : 1;  // zero (reduce complexity)
  std::uint32_t tw    : 1;  // zero (reduce complexity)
  std::uint32_t tsr   : 1;  // zero (reduce complexity)
  std::uint32_t wpri3 : 8;  // zero
  std::uint32_t sd    : 1;  // zero (&xs || &fs)
};

// machine ISA register
struct MISA {
  std::uint32_t ext   : 26; // IMA, read only
  std::uint32_t wlrl  : 4;  // zero
  std::uint32_t mxl   : 2;  // 1, read only
};

// machine interrupt-enable register
struct MIE {
  std::uint32_t usie  : 1;  // zero (no N extension)
  std::uint32_t ssie  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri0 : 1;  // zero
  std::uint32_t msie  : 1;  // M-mode software interrupt enable
  std::uint32_t utie  : 1;  // zero (no N extension)
  std::uint32_t stie  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri1 : 1;  // zero
  std::uint32_t mtie  : 1;  // M-mode timer interrupt enable
  std::uint32_t ueie  : 1;  // zero (no N extension)
  std::uint32_t seie  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri2 : 1;  // zero
  std::uint32_t meie  : 1;  // M-mode external interrupt enable
  std::uint32_t wpri3 : 20; // zero
};

// machine interrupt-pending register
struct MIP {
  std::uint32_t usip  : 1;  // zero (no N extension)
  std::uint32_t ssip  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri0 : 1;  // zero
  std::uint32_t msip  : 1;  // M-mode software interrupt enable
  std::uint32_t utip  : 1;  // zero (no N extension)
  std::uint32_t stip  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri1 : 1;  // zero
  std::uint32_t mtip  : 1;  // M-mode timer interrupt enable
  std::uint32_t ueip  : 1;  // zero (no N extension)
  std::uint32_t seip  : 1;  // zero (no S-mode trap)
  std::uint32_t wpri2 : 1;  // zero
  std::uint32_t meip  : 1;  // M-mode external interrupt enable
  std::uint32_t wpri3 : 20; // zero
};

// machine trap-vector base-address register
struct MTVec {
  std::uint32_t mode  : 2;  // vector mode
  std::uint32_t base  : 30; // base address
};

//  machine cause register
struct MCause {
  std::uint32_t code  : 31; // exception code
  std::uint32_t intr  : 1;  // interrupt flag
};


// privilege levels
constexpr std::uint32_t kPrivLevelU     = 0b00;
constexpr std::uint32_t kPrivLevelS     = 0b01;
constexpr std::uint32_t kPrivLevelM     = 0b11;

// user counters
constexpr std::uint32_t kCSRCycle       = 0xc00;
constexpr std::uint32_t kCSRTime        = 0xc01;
constexpr std::uint32_t kCSRInstRet     = 0xc02;
constexpr std::uint32_t kCSRCycleH      = 0xc80;
constexpr std::uint32_t kCSRTimeH       = 0xc81;
constexpr std::uint32_t kCSRInstRetH    = 0xc82;

// supervisor trap setup
constexpr std::uint32_t kCSRSStatus     = 0x100;
constexpr std::uint32_t kCSRSIE         = 0x104;
constexpr std::uint32_t kCSRSTVec       = 0x105;
constexpr std::uint32_t kCSRSCounterEn  = 0x106;

// supervisor trap handling
constexpr std::uint32_t kCSRSScratch    = 0x140;
constexpr std::uint32_t kCSRSEPC        = 0x141;
constexpr std::uint32_t kCSRSCause      = 0x142;
constexpr std::uint32_t kCSRSTVal       = 0x143;
constexpr std::uint32_t kCSRSIP         = 0x144;

// supervisor protection and translation
constexpr std::uint32_t kCSRSATP        = 0x180;

// machine information registers (read only)
constexpr std::uint32_t kCSRMVenderId   = 0xf11;
constexpr std::uint32_t kCSRMArchId     = 0xf12;
constexpr std::uint32_t kCSRMImpId      = 0xf13;
constexpr std::uint32_t kCSRMHartId     = 0xf14;

// machine trap setup
constexpr std::uint32_t kCSRMStatus     = 0x300;
constexpr std::uint32_t kCSRMISA        = 0x301;
constexpr std::uint32_t kCSRMEDeleg     = 0x302;
constexpr std::uint32_t kCSRMIDeleg     = 0x303;
constexpr std::uint32_t kCSRMIE         = 0x304;
constexpr std::uint32_t kCSRMTVec       = 0x305;
constexpr std::uint32_t kCSRMCounterEn  = 0x306;

// machine trap handling
constexpr std::uint32_t kCSRMScratch    = 0x340;
constexpr std::uint32_t kCSRMEPC        = 0x341;
constexpr std::uint32_t kCSRMCause      = 0x342;
constexpr std::uint32_t kCSRMTVal       = 0x343;
constexpr std::uint32_t kCSRMIP         = 0x344;

// machine memory protection
constexpr std::uint32_t kCSRPMPCfg0     = 0x3a0;
constexpr std::uint32_t kCSRPMPCfg1     = 0x3a1;
constexpr std::uint32_t kCSRPMPCfg2     = 0x3a2;
constexpr std::uint32_t kCSRPMPCfg3     = 0x3a3;
constexpr std::uint32_t kCSRPMPAddr0    = 0x3b0;
constexpr std::uint32_t kCSRPMPAddr1    = 0x3b1;
constexpr std::uint32_t kCSRPMPAddr2    = 0x3b2;
constexpr std::uint32_t kCSRPMPAddr3    = 0x3b3;
constexpr std::uint32_t kCSRPMPAddr4    = 0x3b4;
constexpr std::uint32_t kCSRPMPAddr5    = 0x3b5;
constexpr std::uint32_t kCSRPMPAddr6    = 0x3b6;
constexpr std::uint32_t kCSRPMPAddr7    = 0x3b7;
constexpr std::uint32_t kCSRPMPAddr8    = 0x3b8;
constexpr std::uint32_t kCSRPMPAddr9    = 0x3b9;
constexpr std::uint32_t kCSRPMPAddr10   = 0x3ba;
constexpr std::uint32_t kCSRPMPAddr11   = 0x3bb;
constexpr std::uint32_t kCSRPMPAddr12   = 0x3bc;
constexpr std::uint32_t kCSRPMPAddr13   = 0x3bd;
constexpr std::uint32_t kCSRPMPAddr14   = 0x3be;
constexpr std::uint32_t kCSRPMPAddr15   = 0x3bf;

// machine counters
constexpr std::uint32_t kMCycle         = 0xb00;
constexpr std::uint32_t kMInstRet       = 0xb02;
constexpr std::uint32_t kMCycleH        = 0xb80;
constexpr std::uint32_t kMInstRetH      = 0xb82;

// machine counter setup
constexpr std::uint32_t kMCountInhibit  = 0x320;

#endif  // RISKY32_DEFINE_CSR_H_
