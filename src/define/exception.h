#ifndef RISKY32_DEFINE_EXCEPTION_H_
#define RISKY32_DEFINE_EXCEPTION_H_

#include <cstdint>

// reset vector of all harts
constexpr std::uint32_t kResetVector          = 0x00001000;

// exception code
constexpr std::uint32_t kExcMSoftInt          = 3;
constexpr std::uint32_t kExcMTimerInt         = 7;
constexpr std::uint32_t kExcMExternalInt      = 11;
constexpr std::uint32_t kExcInstAddrMisalign  = 0;
constexpr std::uint32_t kExcInstAccFault      = 1;
constexpr std::uint32_t kExcIllegalInst       = 2;
constexpr std::uint32_t kExcBreakpoint        = 3;
constexpr std::uint32_t kExcLoadAddrMisalign  = 4;
constexpr std::uint32_t kExcLoadAccFault      = 5;
constexpr std::uint32_t kExcStAMOAddrMisalign = 6;
constexpr std::uint32_t kExcStAMOAccFault     = 7;
constexpr std::uint32_t kExcMEnvCall          = 11;
constexpr std::uint32_t kExcInstPageFault     = 12;
constexpr std::uint32_t kExcLoadPageFault     = 13;
constexpr std::uint32_t kExcStAMOPageFault    = 15;

#endif  // RISKY32_DEFINE_EXCEPTION_H_
