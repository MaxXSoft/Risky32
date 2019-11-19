#ifndef RISKY32_DEFINE_CSR_H_
#define RISKY32_DEFINE_CSR_H_

#include <cstdint>

// machine information registers (read only)
constexpr std::uint32_t kCSRMVenderId   = 0xf11;
constexpr std::uint32_t kCSRMArchId     = 0xf12;
constexpr std::uint32_t kCSRMImpId      = 0xf13;
constexpr std::uint32_t kCSRMHartId     = 0xf14;

// machine trap setup (read/write)
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

#endif  // RISKY32_DEFINE_CSR_H_
