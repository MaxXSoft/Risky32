#include "bus/mmu.h"

#include "define/csr.h"
#include "util/cast.h"

/*

Sv32 address translation algorithm:

  PGSIZE = 2 ^ 12, LVLS = 2, PTESIZE = 4
  let a = satp.ppn * PGSIZE
  let i = LVLS - 1

  repeat:
    let pte = mem[a + va.vpn[i] * PTESIZE]

    if pte.v == 0 || (pte.r == 0 && pte.w == 1):
      raise page-fault

    if pte.r == 1 || pte.x == 1:
      break

    i = i - 1
    if i < 0:
      raise page-fault

    a = pte.ppn * PGSIZE


  if check pte.property failed:
    raise page-fault

  if i > 0 && pte.ppn[i - 1 : 0] != 0:
    raise page-fault

  if pte.a == 0 || (is-store && pte.d == 0):
    raise page-fault

  let pa.offset = va.offset,
      pa.ppn[LVLS - 1 : i] = pte.ppn[LVLS - 1 : i],
      if i > 0:
        pa.ppn[i - 1 : 0] = va.vpn[i - 1 : 0]

  return pa

Unrolled version (unrolled by MaxXing):

  PGSIZE = 2 ^ 12, LVLS = 2, PTESIZE = 4
  let a = satp.ppn * PGSIZE
  let pte = mem[a + va.vpn[1] * PTESIZE]

  if pte.v == 0 || (pte.r == 0 && pte.w == 1):
    raise page-fault

  if pte.r == 0 && pte.x == 0:
    a = pte.ppn * PGSIZE
    let pte = mem[a + va.vpn[0] * PTESIZE]

    if pte.v == 0 || (pte.r == 0 && pte.w == 1):
      raise page-fault

    if pte.r == 0 && pte.x == 0:
      raise page-fault

    if check pte.property failed:
      raise page-fault

    if pte.a == 0 || (is-store && pte.d == 0):
      raise page-fault

    let pa.offset = va.offset,
        pa.ppn[1 : 0] = pte.ppn[1 : 0]
    return pa

  else:
    if check pte.property failed:
      raise page-fault

    if pte.ppn[0] != 0:
      raise page-fault

    if pte.a == 0 || (is-store && pte.d == 0):
      raise page-fault

    let pa.offset = va.offset,
        pa.ppn[1] = pte.ppn[1],
        pa.ppn[0] = va.vpn[0]
    return pa

*/

#define PAGE_FAULT      \
  do {                  \
    is_invalid_ = true; \
    return 0;           \
  } while (0)

std::uint32_t MMU::GetPhysicalAddr(std::uint32_t addr, bool is_store,
                                   bool is_execute) {
  last_vaddr_ = addr;
  auto satp_val = csr_.satp();
  auto satp = PtrCast<SATP>(&satp_val);
  if (csr_.cur_priv() == kPrivLevelM || !satp->mode) {
    // address translation is not enabled
    return addr;
  }
  else {
    auto va = PtrCast<Sv32VAddr>(&addr);
    // read first page table entry from bus
    auto pte_addr = (satp->ppn << 12) + (va->vpn1) * 4;
    auto pte_val = bus_->ReadWord(pte_addr);
    auto pte = PtrCast<Sv32PTE>(&pte_val);
    // check if is valid PTE
    if (!pte->v || (!pte->r && pte->w)) PAGE_FAULT;
    // check if PTE is a pointer
    if (!pte->r && !pte->x) {
      // read second page table entry from bus
      auto pte_ppn = (pte->ppn1 << 10) | pte->ppn0;
      pte_addr = (pte_ppn << 12) + (va->vpn0) * 4;
      pte_val = bus_->ReadWord(pte_addr);
      // check if is a valid PTE
      if (!pte->v || (!pte->r && pte->w)) PAGE_FAULT;
      if (!pte->r && !pte->x) PAGE_FAULT;
      if (!CheckPTEProperty(*pte, is_store, is_execute)) PAGE_FAULT;
      if (!pte->a || (is_store && !pte->d)) PAGE_FAULT;
      // get physical address
      auto pa = (pte_ppn << 12) | va->offset;
      return pa;
    }
    else {
      // check if is a valid PTE
      if (!CheckPTEProperty(*pte, is_store, is_execute)) PAGE_FAULT;
      if (pte->ppn0) PAGE_FAULT;
      if (!pte->a || (is_store && !pte->d)) PAGE_FAULT;
      // get physical address
      auto pa = (pte->ppn1 << 22) | (va->vpn0 << 12) | va->offset;
      return pa;
    }
  }
}

bool MMU::CheckPTEProperty(const Sv32PTE &pte, bool is_store,
                           bool is_execute) {
  if (!is_store && !is_execute && !pte.r) return false;
  if (is_store && !pte.w) return false;
  if (is_execute && !pte.x) return false;
  if (csr_.cur_priv() == kPrivLevelS && pte.u) return false;
  return true;
}

std::uint8_t MMU::ReadByte(std::uint32_t addr) {
  if (is_invalid_) return 0;
  auto pa = GetPhysicalAddr(addr, false, false);
  return is_invalid_ ? 0 : bus_->ReadByte(pa);
}

void MMU::WriteByte(std::uint32_t addr, std::uint8_t value) {
  if (is_invalid_) return;
  auto pa = GetPhysicalAddr(addr, true, false);
  if (!is_invalid_) {
    bus_->WriteByte(pa, value);
  }
}

std::uint16_t MMU::ReadHalf(std::uint32_t addr) {
  if (is_invalid_) return 0;
  auto pa = GetPhysicalAddr(addr, false, false);
  return is_invalid_ ? 0 : bus_->ReadHalf(pa);
}

void MMU::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  if (is_invalid_) return;
  auto pa = GetPhysicalAddr(addr, true, false);
  if (!is_invalid_) {
    bus_->WriteHalf(pa, value);
  }
}

std::uint32_t MMU::ReadWord(std::uint32_t addr) {
  if (is_invalid_) return 0;
  auto pa = GetPhysicalAddr(addr, false, false);
  return is_invalid_ ? 0 : bus_->ReadWord(pa);
}

void MMU::WriteWord(std::uint32_t addr, std::uint32_t value) {
  if (is_invalid_) return;
  auto pa = GetPhysicalAddr(addr, true, false);
  if (!is_invalid_) {
    bus_->WriteWord(pa, value);
  }
}

std::uint32_t MMU::ReadInst(std::uint32_t addr) {
  if (is_invalid_) return 0;
  auto pa = GetPhysicalAddr(addr, false, true);
  return is_invalid_ ? 0 : bus_->ReadWord(pa);
}
