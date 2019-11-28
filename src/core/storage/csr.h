#ifndef RISKY32_CORE_STORAGE_CSR_H_
#define RISKY32_CORE_STORAGE_CSR_H_

#include <unordered_map>
#include <cstdint>

#include "define/csr.h"
#include "util/cast.h"

// control and status registers
class CSR {
 public:
  CSR();

  // update performance counter
  void UpdateCounter();
  // read data from CSR, returns false if failed
  bool ReadData(std::uint32_t addr, std::uint32_t &value);
  // write data to CSR, returns false if failed
  bool WriteData(std::uint32_t addr, std::uint32_t value);

  // setters
  void set_cur_priv(std::uint32_t cur_priv) { cur_priv_ = cur_priv; }
  void set_mepc(std::uint32_t mepc) { mepc_ = mepc; }
  void set_mcause(std::uint32_t mcause) { mcause_ = mcause; }
  void set_mtval(std::uint32_t mtval) { mtval_ = mtval; }

  // getters
  // current privilege level
  std::uint32_t cur_priv() const { return cur_priv_; }
  // S-mode exception program counter
  std::uint32_t sepc() const { return sepc_; }
  // trap vector
  std::uint32_t trap_vec() const {
    if (mtvec_ & 0b11 == 1) {
      return mtvec_ - 1 + (mcause_ & 0x7fffffff) * 4;
    }
    else {
      return mtvec_;
    }
  }
  // M-mode exception program counter
  std::uint32_t mepc() const { return mepc_; }
  // M-mode exception cause
  std::uint32_t mcause() const { return mcause_; }

 private:
  void InitCSR();
  void InitMapping();

  // current privilege level
  std::uint32_t cur_priv_;
  // CSR address mapping
  std::unordered_map<std::uint32_t, std::uint32_t *> csrs_;
  // CSR that hardwired to zero
  std::uint32_t zero_;
  // supervisor mode CSRs
  std::uint32_t sstatus_, sscratch_, sepc_, satp_;
  // machine mode CSRs
  std::uint32_t mstatus_, misa_, mie_, mtvec_, mscratch_;
  std::uint32_t mepc_, mcause_, mtval_, mip_;
  // machine mode counters (64-bit)
  std::uint64_t mcycle_, minstret_;
};

#endif  // RISKY32_CORE_STORAGE_CSR_H_
