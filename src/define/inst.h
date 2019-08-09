#ifndef RISKY32_DEFINE_INST_H_
#define RISKY32_DEFINE_INST_H_

#include <cstdint>

// instruction
struct Inst {
  std::uint32_t opcode  : 7;
  std::uint32_t data    : 25;
};

// R-type instruction
struct InstR {
  std::uint32_t opcode  : 7;
  std::uint32_t rd      : 5;
  std::uint32_t funct3  : 3;
  std::uint32_t rs1     : 5;
  std::uint32_t rs2     : 5;
  std::uint32_t funct7  : 7;
};

// I-type instruction
struct InstI {
  std::uint32_t opcode  : 7;
  std::uint32_t rd      : 5;
  std::uint32_t funct3  : 3;
  std::uint32_t rs1     : 5;
  std::uint32_t imm     : 12;
};

// S-type instruction
struct InstS {
  std::uint32_t opcode  : 7;
  std::uint32_t imm5    : 5;
  std::uint32_t funct3  : 3;
  std::uint32_t rs1     : 5;
  std::uint32_t rs2     : 5;
  std::uint32_t imm7    : 7;
};

// U-type instruction
struct InstU {
  std::uint32_t opcode  : 7;
  std::uint32_t rd      : 5;
  std::uint32_t imm     : 20;
};

// 'opcode' field
constexpr std::uint32_t kLoad     = 0b0000011;
constexpr std::uint32_t kLoadFP   = 0b0000111;
constexpr std::uint32_t kMiscMem  = 0b0001111;
constexpr std::uint32_t kOpImm    = 0b0010011;
constexpr std::uint32_t kAUIPC    = 0b0010111;
constexpr std::uint32_t kStore    = 0b0100011;
constexpr std::uint32_t kStoreFP  = 0b0100111;
constexpr std::uint32_t kAMO      = 0b0101111;
constexpr std::uint32_t kOp       = 0b0110011;
constexpr std::uint32_t kLUI      = 0b0110111;
constexpr std::uint32_t kMADD     = 0b1000011;
constexpr std::uint32_t kMSUB     = 0b1000111;
constexpr std::uint32_t kNMSUB    = 0b1001011;
constexpr std::uint32_t kNMADD    = 0b1001111;
constexpr std::uint32_t kOpFP     = 0b1010011;
constexpr std::uint32_t kBranch   = 0b1100011;
constexpr std::uint32_t kJALR     = 0b1100111;
constexpr std::uint32_t kJAL      = 0b1101111;
constexpr std::uint32_t kSystem   = 0b1110011;

// 'funct3' field used to represent 'width'
constexpr std::uint32_t kWord     = 0b010;
constexpr std::uint32_t kDouble   = 0b011;

// 'funct3' field in 'LOAD' instructions
constexpr std::uint32_t kLB       = 0b000;
constexpr std::uint32_t kLH       = 0b001;
constexpr std::uint32_t kLW       = 0b010;
constexpr std::uint32_t kLBU      = 0b100;
constexpr std::uint32_t kLHU      = 0b101;

// 'funct3' field in 'MISC-MEM' instructions
constexpr std::uint32_t kFENCE    = 0b000;
constexpr std::uint32_t kFENCEI   = 0b001;

// 'funct3' field in 'OP-IMM' instructions
constexpr std::uint32_t kADDI     = 0b000;
constexpr std::uint32_t kSLLI     = 0b001;
constexpr std::uint32_t kSLTI     = 0b010;
constexpr std::uint32_t kSLTIU    = 0b011;
constexpr std::uint32_t kXORI     = 0b100;
constexpr std::uint32_t kSRXI     = 0b101;  // SRLI/SRAI
constexpr std::uint32_t kORI      = 0b110;
constexpr std::uint32_t kANDI     = 0b111;

// 'funct3' field in 'STORE' instructions
constexpr std::uint32_t kSB       = 0b000;
constexpr std::uint32_t kSH       = 0b001;
constexpr std::uint32_t kSW       = 0b010;

// 'funct7' field in 'AMO' instructions (ignored ordering bits)
constexpr std::uint32_t kLR       = 0b0001000;
constexpr std::uint32_t kSC       = 0b0001100;
constexpr std::uint32_t kAMOSWAP  = 0b0000100;
constexpr std::uint32_t kAMOADD   = 0b0000000;
constexpr std::uint32_t kAMOXOR   = 0b0010000;
constexpr std::uint32_t kAMOAND   = 0b0110000;
constexpr std::uint32_t kAMOOR    = 0b0100000;
constexpr std::uint32_t kAMOMIN   = 0b1000000;
constexpr std::uint32_t kAMOMAX   = 0b1010000;
constexpr std::uint32_t kAMOMINU  = 0b1100000;
constexpr std::uint32_t kAMOMAXU  = 0b1110000;

// 'funct3' field in 'OP' instructions
constexpr std::uint32_t kADDSUB   = 0b000;  // ADD/SUB
constexpr std::uint32_t kSLL      = 0b001;
constexpr std::uint32_t kSLT      = 0b010;
constexpr std::uint32_t kSLTU     = 0b011;
constexpr std::uint32_t kXOR      = 0b100;
constexpr std::uint32_t kSRX      = 0b101;  // SRL/SRA
constexpr std::uint32_t kOR       = 0b110;
constexpr std::uint32_t kAND      = 0b111;
constexpr std::uint32_t kMUL      = 0b000;
constexpr std::uint32_t kMULH     = 0b001;
constexpr std::uint32_t kMULHSU   = 0b010;
constexpr std::uint32_t kMULHU    = 0b011;
constexpr std::uint32_t kDIV      = 0b100;
constexpr std::uint32_t kDIVU     = 0b101;
constexpr std::uint32_t kREM      = 0b110;
constexpr std::uint32_t kREMU     = 0b111;

// 'funct7' field in 'OP' instructions
constexpr std::uint32_t kRV32I1   = 0b0000000;
constexpr std::uint32_t kRV32I2   = 0b0100000;
constexpr std::uint32_t kRV32M    = 0b0000001;

// fields in 'OpFP' instructions
// TODO

// 'funct3' field in 'BRANCH' instructions
constexpr std::uint32_t kBEQ      = 0b000;
constexpr std::uint32_t kBNE      = 0b001;
constexpr std::uint32_t kBLT      = 0b100;
constexpr std::uint32_t kBGE      = 0b101;
constexpr std::uint32_t kBLTU     = 0b110;
constexpr std::uint32_t kBGEU     = 0b111;

// 'funct3' field in 'SYSTEM' instructions
constexpr std::uint32_t kPRIV     = 0b000;  // ECALL/EBREAK/MRET/WFI
constexpr std::uint32_t kCSRRW    = 0b001;
constexpr std::uint32_t kCSRRS    = 0b010;
constexpr std::uint32_t kCSRRC    = 0b011;
constexpr std::uint32_t kCSRRWI   = 0b101;
constexpr std::uint32_t kCSRRSI   = 0b110;
constexpr std::uint32_t kCSRRCI   = 0b111;

// 'imm' field in 'SYSTEM' instructions
constexpr std::uint32_t kECALL    = 0b000000000000;
constexpr std::uint32_t kEBREAK   = 0b000000000001;
constexpr std::uint32_t kMRET     = 0b001100000010;
constexpr std::uint32_t kWFI      = 0b000100000101;

#endif  // RISKY32_DEFINE_INST_H_
