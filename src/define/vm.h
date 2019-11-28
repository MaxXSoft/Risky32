#ifndef RISKY32_DEFINE_VM_H_
#define RISKY32_DEFINE_VM_H_

#include <cstdint>

// Sv32 virtual address
struct Sv32VAddr {
  std::uint32_t offset  : 12; // page offset
  std::uint32_t vpn0    : 10; // virtual page number 0
  std::uint32_t vpn1    : 10; // virtual page number 1
};

// Sv32 page table entry
struct Sv32PTE {
  std::uint32_t v       : 1;  // valid
  std::uint32_t r       : 1;  // readable
  std::uint32_t w       : 1;  // writable
  std::uint32_t x       : 1;  // executable
  std::uint32_t u       : 1;  // user accessible
  std::uint32_t g       : 1;  // global mapping
  std::uint32_t a       : 1;  // accessed
  std::uint32_t d       : 1;  // dirty
  std::uint32_t rsw     : 2;  // reversed for S-mode software
  std::uint32_t ppn0    : 10; // physical page number 0
  std::uint32_t ppn1    : 12; // physical page number 1
};

#endif  // RISKY32_DEFINE_VM_H_
