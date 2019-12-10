#ifndef RISKY32_DEBUGGER_DISASM_H_
#define RISKY32_DEBUGGER_DISASM_H_

#include <string>
#include <utility>
#include <cstdint>

using Disasm = std::pair<std::string, std::string>;

// get disassembled instruction data
Disasm Disassemble(std::uint32_t inst_data, std::uint32_t addr);

#endif  // RISKY32_DEBUGGER_DISASM_H_
