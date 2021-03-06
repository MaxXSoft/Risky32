#ifndef RISKY32_DEFINE_EXCEPTION_H_
#define RISKY32_DEFINE_EXCEPTION_H_

#include <cstdint>

/*

NOTE:
  access fault: trying to access memory without permission
  page fault:   trying to access page that does not exists

*/

// reset vectors of all harts
constexpr std::uint32_t kTrapVector           = 0x00000100;
constexpr std::uint32_t kResetVector          = 0x00000200;

// exception code
constexpr std::uint32_t kExcMSoftInt          = 3;
constexpr std::uint32_t kExcMTimerInt         = 7;
constexpr std::uint32_t kExcMExternalInt      = 11;
constexpr std::uint32_t kExcInstAddrMisalign  = 0;
constexpr std::uint32_t kExcInstAccFault      = 1;    // ignored
constexpr std::uint32_t kExcIllegalInst       = 2;
constexpr std::uint32_t kExcBreakpoint        = 3;
constexpr std::uint32_t kExcLoadAddrMisalign  = 4;
constexpr std::uint32_t kExcLoadAccFault      = 5;    // ignored
constexpr std::uint32_t kExcStAMOAddrMisalign = 6;
constexpr std::uint32_t kExcStAMOAccFault     = 7;    // ignored
constexpr std::uint32_t kExcUEnvCall          = 8;
constexpr std::uint32_t kExcSEnvCall          = 9;
constexpr std::uint32_t kExcMEnvCall          = 11;
constexpr std::uint32_t kExcInstPageFault     = 12;
constexpr std::uint32_t kExcLoadPageFault     = 13;
constexpr std::uint32_t kExcStAMOPageFault    = 15;

#endif  // RISKY32_DEFINE_EXCEPTION_H_
