#include "debugger/disasm.h"

#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <cassert>

#include "define/inst.h"
#include "define/csr.h"
#include "util/bitvalue.h"
#include "util/bitpat.h"

namespace {

// immediate encoding variants
enum class ImmEncode {
  R,  // no immediate
  I,  // inst[31:20]
  S,  // {inst[31:25], inst[11:7]}
  B,  // {inst[31], inst[7], inst[30:25], inst[11:8], 1'b0}
  U,  // {inst[31:12]}
  J,  // {inst[31], inst[19:12], inst[20], inst[30:21], 1'b0}
};

// assembly format
enum class AsmFormat {
  None,         // ECALL, FENCE.I
  RegRegReg,    // normal R-type
  RegRegImm,    // normal I-type
  RegRegSmt,    // SLLI
  RegReg,       // SFENCE.VMA
  RegImm,       // LUI
  RegTarget,    // JAL
  RegRegTarget, // BEQ
  RegBaseImm,   // JALR, LW
  MemOrder,     // FENCE
  AMO2,         // LR.W
  AMO3,         // SC.W, AMOSWAP.W
  CSRReg,       // CSRRW
  CSRImm,       // CSRRWI
};

// assembly info
struct AsmInfo {
  std::string_view  opcode;
  ImmEncode         imm;
  AsmFormat         format;
};

// argument of assembly formatter
struct AsmArgs {
  std::uint32_t     rd;
  std::uint32_t     rs1;
  std::uint32_t     rs2;
  std::uint32_t     imm;
};

// map of GPR/CSR address to name
const std::unordered_map<std::uint32_t, std::string_view> kNameMap = {
  // GPRs
  {0,   "zero"},  {1,   "ra"},  {2,   "sp"},  {3,   "gp"},
  {4,   "tp"},    {5,   "t0"},  {6,   "t1"},  {7,   "t2"},
  {8,   "fp"},    {9,   "s1"},  {10,  "a0"},  {11,  "a1"},
  {12,  "a2"},    {13,  "a3"},  {14,  "a4"},  {15,  "a5"},
  {16,  "a6"},    {17,  "a7"},  {18,  "s2"},  {19,  "s3"},
  {20,  "s4"},    {21,  "s5"},  {22,  "s6"},  {23,  "s7"},
  {24,  "s8"},    {25,  "s9"},  {26,  "s10"}, {27,  "s11"},
  {28,  "t3"},    {29,  "t4"},  {30,  "t5"},  {31,  "t6"},
  // U-mode CSRs
  {kCSRCycle, "cycle"}, {kCSRInstRet, "instret"},
  {kCSRCycleH, "cycleh"}, {kCSRInstRetH, "instreth"},
  // S-mode CSRs
  {kCSRSStatus, "sstatus"}, {kCSRSIE, "sie"}, {kCSRSTVec, "stvec"},
  {kCSRSCounterEn, "scounteren"}, {kCSRSScratch, "sscratch"},
  {kCSRSEPC, "sepc"}, {kCSRSCause, "scause"}, {kCSRSTVal, "stval"},
  {kCSRSIP, "sip"}, {kCSRSATP, "satp"},
  // M-mode CSRs
  {kCSRMVenderId, "mvenderid"}, {kCSRMArchId, "marchid"},
  {kCSRMImpId, "mimpid"}, {kCSRMHartId, "mhartid"},
  {kCSRMStatus, "mstatus"}, {kCSRMISA, "misa"}, {kCSRMIE, "mie"},
  {kCSRMTVec, "mtvec"}, {kCSRMCounterEn, "mcounteren"},
  {kCSRMScratch, "mscratch"}, {kCSRMEPC, "mepc"}, {kCSRMCause, "mcause"},
  {kCSRMTVal, "mtval"}, {kCSRMIP, "mip"},
  {kCSRPMPCfg0, "pmpcfg0"}, {kCSRPMPCfg1, "pmpcfg1"},
  {kCSRPMPCfg2, "pmpcfg2"}, {kCSRPMPCfg3, "pmpcfg3"},
  {kCSRPMPAddr0, "pmpaddr0"}, {kCSRPMPAddr1, "pmpaddr1"},
  {kCSRPMPAddr2, "pmpaddr2"}, {kCSRPMPAddr3, "pmpaddr3"},
  {kCSRPMPAddr4, "pmpaddr4"}, {kCSRPMPAddr5, "pmpaddr5"},
  {kCSRPMPAddr6, "pmpaddr6"}, {kCSRPMPAddr7, "pmpaddr7"},
  {kCSRPMPAddr8, "pmpaddr8"}, {kCSRPMPAddr9, "pmpaddr9"},
  {kCSRPMPAddr10, "pmpaddr10"}, {kCSRPMPAddr11, "pmpaddr11"},
  {kCSRPMPAddr12, "pmpaddr12"}, {kCSRPMPAddr13, "pmpaddr13"},
  {kCSRPMPAddr14, "pmpaddr14"}, {kCSRPMPAddr15, "pmpaddr15"},
  {kCSRMCycle, "mcycle"}, {kCSRMInstRet, "minstret"},
  {kCSRMCycleH, "mcycleh"}, {kCSRMInstRetH, "minstreth"},
  {kCSRMCountInhibit, "mcountinhibit"},
};

// bitpat map of opcodes
BitMatch32<AsmInfo> kOpMap = {
  // arithmetic
  {"0000000??????????000?????0110011", {"add", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????000?????0010011", {"addi", ImmEncode::I, AsmFormat::RegRegImm}},
  {"0100000??????????000?????0110011", {"sub", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????????????0110111", {"lui", ImmEncode::U, AsmFormat::RegImm}},
  {"?????????????????????????0010111", {"auipc", ImmEncode::U, AsmFormat::RegImm}},
  // logical
  {"0000000??????????100?????0110011", {"xor", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????100?????0010011", {"xori", ImmEncode::I, AsmFormat::RegRegImm}},
  {"0000000??????????110?????0110011", {"or", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????110?????0010011", {"ori", ImmEncode::I, AsmFormat::RegRegImm}},
  {"0000000??????????111?????0110011", {"and", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????111?????0010011", {"andi", ImmEncode::I, AsmFormat::RegRegImm}},
  // compare
  {"0000000??????????010?????0110011", {"slt", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????010?????0010011", {"slti", ImmEncode::I, AsmFormat::RegRegImm}},
  {"0000000??????????011?????0110011", {"sltu", ImmEncode::R, AsmFormat::RegRegReg}},
  {"?????????????????011?????0010011", {"sltiu", ImmEncode::I, AsmFormat::RegRegImm}},
  // shift
  {"0000000??????????001?????0110011", {"sll", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000000??????????001?????0010011", {"slli", ImmEncode::R, AsmFormat::RegRegSmt}},
  {"0000000??????????101?????0110011", {"srl", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000000??????????101?????0010011", {"srli", ImmEncode::R, AsmFormat::RegRegSmt}},
  {"0100000??????????101?????0110011", {"sra", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0100000??????????101?????0010011", {"srai", ImmEncode::R, AsmFormat::RegRegSmt}},
  // branch & jump
  {"?????????????????000?????1100011", {"beq", ImmEncode::B, AsmFormat::RegRegTarget}},
  {"?????????????????001?????1100011", {"bne", ImmEncode::B, AsmFormat::RegRegTarget}},
  {"?????????????????100?????1100011", {"blt", ImmEncode::B, AsmFormat::RegRegTarget}},
  {"?????????????????101?????1100011", {"bge", ImmEncode::B, AsmFormat::RegRegTarget}},
  {"?????????????????110?????1100011", {"bltu", ImmEncode::B, AsmFormat::RegRegTarget}},
  {"?????????????????111?????1100011", {"bgeu", ImmEncode::B, AsmFormat::RegRegTarget}},
  {"?????????????????????????1101111", {"jal", ImmEncode::J, AsmFormat::RegTarget}},
  {"?????????????????000?????1100111", {"jalr", ImmEncode::I, AsmFormat::RegBaseImm}},
  // load & store
  {"?????????????????000?????0000011", {"lb", ImmEncode::I, AsmFormat::RegBaseImm}},
  {"?????????????????001?????0000011", {"lh", ImmEncode::I, AsmFormat::RegBaseImm}},
  {"?????????????????010?????0000011", {"lw", ImmEncode::I, AsmFormat::RegBaseImm}},
  {"?????????????????100?????0000011", {"lbu", ImmEncode::I, AsmFormat::RegBaseImm}},
  {"?????????????????101?????0000011", {"lhu", ImmEncode::I, AsmFormat::RegBaseImm}},
  {"?????????????????000?????0100011", {"sb", ImmEncode::S, AsmFormat::RegBaseImm}},
  {"?????????????????001?????0100011", {"sh", ImmEncode::S, AsmFormat::RegBaseImm}},
  {"?????????????????010?????0100011", {"sw", ImmEncode::S, AsmFormat::RegBaseImm}},
  // sync
  {"0000????????00000000000000001111", {"fence", ImmEncode::I, AsmFormat::MemOrder}},
  {"00000000000000000001000000001111", {"fence.i", ImmEncode::I, AsmFormat::None}},
  // CSR access
  {"?????????????????001?????1110011", {"csrrw", ImmEncode::I, AsmFormat::CSRReg}},
  {"?????????????????010?????1110011", {"csrrs", ImmEncode::I, AsmFormat::CSRReg}},
  {"?????????????????011?????1110011", {"csrrc", ImmEncode::I, AsmFormat::CSRReg}},
  {"?????????????????101?????1110011", {"csrrwi", ImmEncode::I, AsmFormat::CSRImm}},
  {"?????????????????110?????1110011", {"csrrsi", ImmEncode::I, AsmFormat::CSRImm}},
  {"?????????????????111?????1110011", {"csrrci", ImmEncode::I, AsmFormat::CSRImm}},
  // multiplication & division
  {"0000001??????????000?????0110011", {"mul", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????001?????0110011", {"mulh", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????010?????0110011", {"mulhsu", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????011?????0110011", {"mulhu", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????100?????0110011", {"div", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????101?????0110011", {"divu", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????110?????0110011", {"rem", ImmEncode::R, AsmFormat::RegRegReg}},
  {"0000001??????????111?????0110011", {"remu", ImmEncode::R, AsmFormat::RegRegReg}},
  // atomic
  {"00010??00000?????010?????0101111", {"lr.w", ImmEncode::R, AsmFormat::AMO2}},
  {"00011????????????010?????0101111", {"sc.w", ImmEncode::R, AsmFormat::AMO3}},
  {"00001????????????010?????0101111", {"amoswap.w", ImmEncode::R, AsmFormat::AMO3}},
  {"00000????????????010?????0101111", {"amoadd.w", ImmEncode::R, AsmFormat::AMO3}},
  {"00100????????????010?????0101111", {"amoxor.w", ImmEncode::R, AsmFormat::AMO3}},
  {"01100????????????010?????0101111", {"amoand.w", ImmEncode::R, AsmFormat::AMO3}},
  {"01000????????????010?????0101111", {"amoor.w", ImmEncode::R, AsmFormat::AMO3}},
  {"10000????????????010?????0101111", {"amomin.w", ImmEncode::R, AsmFormat::AMO3}},
  {"10100????????????010?????0101111", {"amomax.w", ImmEncode::R, AsmFormat::AMO3}},
  {"11000????????????010?????0101111", {"amominu.w", ImmEncode::R, AsmFormat::AMO3}},
  {"11100????????????010?????0101111", {"amomaxu.w", ImmEncode::R, AsmFormat::AMO3}},
  // privilege
  {"00000000000000000000000001110011", {"ecall", ImmEncode::I, AsmFormat::None}},
  {"00000000000100000000000001110011", {"ebreak", ImmEncode::I, AsmFormat::None}},
  {"00010000001000000000000001110011", {"sret", ImmEncode::R, AsmFormat::None}},
  {"00110000001000000000000001110011", {"mret", ImmEncode::R, AsmFormat::None}},
  {"00010000010100000000000001110011", {"wfi", ImmEncode::R, AsmFormat::None}},
  {"0001001??????????000000001110011", {"sfence.vma", ImmEncode::R, AsmFormat::RegReg}},
  // pseudo instruction
  {"00000000000000000000000000010011", {"nop", ImmEncode::I, AsmFormat::None}},
  // unknown
  {"????????????????????????????????", {"unimp", ImmEncode::R, AsmFormat::None}},
};

// extract immediate from instruction data by specific immediate encoding
std::uint32_t GetImmFromInst(std::uint32_t inst_data, ImmEncode imm_enc) {
  BitValue32 bv = {inst_data, 32};
  BitValue32 z = {0, 1};
  switch (imm_enc) {
    case ImmEncode::I: return bv(31, 20);
    case ImmEncode::S: return bv(31, 25) | bv(11, 7);
    case ImmEncode::B: return bv(31) | bv(7) | bv(30, 25) | bv(11, 8) | z;
    case ImmEncode::U: return bv(31, 12);
    case ImmEncode::J: return bv(31) | bv(19, 12) | bv(20) | bv(30, 21) | z;
    default: return 0;
  }
}

// print register name to output stream
void PrintRegName(std::ostream &os, std::uint32_t addr) {
  auto it = kNameMap.find(addr);
  if (it != kNameMap.end()) {
    os << it->second;
  }
  else {
    os << "0x" << std::hex << std::setw(3) << std::setfill('0') << addr;
  }
}

// print memory order to output stream
void PrintOrder(std::ostream &os, const BitValue32 &order) {
  assert(order.width() == 4);
  if (!order) {
    os << "unknown";
  }
  else {
    if (order[3]) os << 'i';
    if (order[2]) os << 'o';
    if (order[1]) os << 'r';
    if (order[0]) os << 'w';
  }
}

AsmArgs GetAsmArgs(std::uint32_t inst_data, const AsmInfo &info) {
  // get bit value of current instruction
  BitValue32 inst = {inst_data, 32};
  // get argument list
  auto imm = GetImmFromInst(inst_data, info.imm);
  return {inst(11, 7), inst(19, 15), inst(24, 20), imm};
}

std::string GetAsmString(const AsmInfo &info, const AsmArgs &args,
                         std::uint32_t addr) {
  std::ostringstream oss;
  switch (info.format) {
    case AsmFormat::RegRegReg: {
      PrintRegName(oss, args.rd);
      oss << ", ";
      PrintRegName(oss, args.rs1);
      oss << ", ";
      PrintRegName(oss, args.rs2);
      break;
    }
    case AsmFormat::RegRegImm: {
      PrintRegName(oss, args.rd);
      oss << ", ";
      PrintRegName(oss, args.rs1);
      oss << ", 0x" << std::hex << args.imm;
      break;
    }
    case AsmFormat::RegRegSmt: {
      PrintRegName(oss, args.rd);
      oss << ", ";
      PrintRegName(oss, args.rs1);
      oss << ", " << args.rs2;
      break;
    }
    case AsmFormat::RegReg: {
      PrintRegName(oss, args.rs1);
      oss << ", ";
      PrintRegName(oss, args.rs2);
      break;
    }
    case AsmFormat::RegImm: {
      PrintRegName(oss, args.rd);
      oss << ", 0x" << std::hex << args.imm;
      break;
    }
    case AsmFormat::RegTarget: {
      PrintRegName(oss, args.rd);
      auto ofs = args.imm & (1 << 20) ? 0xffe00000 | args.imm : args.imm;
      oss << ", 0x" << std::hex << (addr + ofs);
      break;
    }
    case AsmFormat::RegRegTarget: {
      PrintRegName(oss, args.rs1);
      oss << ", ";
      PrintRegName(oss, args.rs2);
      auto ofs = args.imm & (1 << 12) ? 0xffffe000 | args.imm : args.imm;
      oss << ", 0x" << std::hex << (addr + ofs);
      break;
    }
    case AsmFormat::RegBaseImm: {
      if (info.imm == ImmEncode::S) {
        PrintRegName(oss, args.rs2);
        oss << ", 0x" << std::hex << args.imm;
        oss << '(';
        PrintRegName(oss, args.rs1);
        oss << ')';
      }
      else {
        PrintRegName(oss, args.rd);
        oss << ", 0x" << std::hex << args.imm;
        oss << '(';
        PrintRegName(oss, args.rs1);
        oss << ')';
      }
      break;
    }
    case AsmFormat::MemOrder: {
      BitValue32 order = {args.imm, 12};
      PrintOrder(oss, order(3, 0));
      oss << ", ";
      PrintOrder(oss, order(7, 4));
      break;
    }
    case AsmFormat::AMO2: {
      PrintRegName(oss, args.rd);
      oss << ", (";
      PrintRegName(oss, args.rs1);
      oss << ')';
      break;
    }
    case AsmFormat::AMO3: {
      PrintRegName(oss, args.rd);
      oss << ", ";
      PrintRegName(oss, args.rs2);
      oss << ", (";
      PrintRegName(oss, args.rs1);
      oss << ')';
      break;
    }
    case AsmFormat::CSRReg: {
      PrintRegName(oss, args.rd);
      oss << ", ";
      PrintRegName(oss, args.imm);
      oss << ", ";
      PrintRegName(oss, args.rs1);
      break;
    }
    case AsmFormat::CSRImm: {
      PrintRegName(oss, args.rd);
      oss << ", ";
      PrintRegName(oss, args.imm);
      oss << ", 0x" << std::hex << args.rs1;
      break;
    }
    default:;
  }
  return oss.str();
}

std::string GetAsmOpcode(std::uint32_t inst_data, const AsmInfo &info) {
  if (info.format == AsmFormat::AMO2 || info.format == AsmFormat::AMO3) {
    // check 'aquire' and 'release' flags
    std::ostringstream oss(info.opcode.data());
    BitValue32 bv = {inst_data, 32};
    if (bv[26]) oss << ".aq";
    if (bv[25]) oss << ".rl";
    return oss.str();
  }
  else {
    return info.opcode.data();
  }
}

}  // namespace

Disasm Disassemble(std::uint32_t inst_data, std::uint32_t addr) {
  // get assembly info
  auto info_opt = kOpMap.Find(inst_data);
  assert(info_opt);
  const auto &info = *info_opt;
  // get opcode string
  auto opcode = GetAsmOpcode(inst_data, info);
  // get format string
  auto args = GetAsmArgs(inst_data, info);
  auto format = GetAsmString(info, args, addr);
  return {opcode, format};
}
