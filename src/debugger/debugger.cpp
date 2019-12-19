#include "debugger/debugger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cctype>

#include "readline/readline.h"
#include "readline/history.h"
#include "debugger/disasm.h"
#include "define/mmio.h"
#include "util/style.h"

namespace {

// MMIO address of 'break' operation
constexpr std::uint32_t kAddrBreak = 0x0;
// debugger breakpoint instruction ('sw zero, 0xff0(zero)')
constexpr std::uint32_t kBreakInst = 0xfe002823;
static_assert(kAddrBreak == 0x0 && kMMIOAddrDebugger == 0xfffffff0);

// name of all debugger commands
enum class CommandName {
  Unknown,
  Help, Quit,
  Break, Watch, Delete,
  Continue, StepInst,
  Print, Examine, Disasm, Info,
};

// name of all available ITEMs in command 'info'
enum class InfoItem {
  Reg, CSR, Break, Watch,
};

// structure of internal disassembly information
struct DisasmInfo {
  bool            is_breakpoint;
  std::uint32_t   addr;
  std::uint32_t   inst_data;
  Disasm          disasm;
};

// hashmap of all commands
const std::unordered_map<std::string_view, CommandName> kCmdMap = {
  {"help", CommandName::Help},
  {"quit", CommandName::Quit}, {"q", CommandName::Quit},
  {"break", CommandName::Break}, {"b", CommandName::Break},
  {"watch", CommandName::Watch}, {"w", CommandName::Watch},
  {"delete", CommandName::Delete}, {"d", CommandName::Delete},
  {"continue", CommandName::Continue}, {"c", CommandName::Continue},
  {"stepi", CommandName::StepInst}, {"si", CommandName::StepInst},
  {"print", CommandName::Print}, {"p", CommandName::Print},
  {"x", CommandName::Examine},
  {"disasm", CommandName::Disasm}, {"da", CommandName::Disasm},
  {"info", CommandName::Info},
};

// hashmap of all ITEMs
const std::unordered_map<std::string_view, InfoItem> kInfoItemMap = {
  {"reg", InfoItem::Reg}, {"r", InfoItem::Reg},
  {"csr", InfoItem::CSR}, {"c", InfoItem::CSR},
  {"break", InfoItem::Break}, {"b", InfoItem::Break},
  {"watch", InfoItem::Watch}, {"w", InfoItem::Watch},
};

// get command name by string
inline CommandName GetCommandName(std::string_view cmd) {
  auto it = kCmdMap.find(cmd);
  return it == kCmdMap.end() ? CommandName::Unknown : it->second;
}

// print error message
inline void LogError(std::string_view msg) {
  std::cout << "ERROR (debugger): " << msg << std::endl;
}

// display help message
void PrintHelp() {
  std::cout << "Debugger commands:" << std::endl;
  std::cout << "  help      [CMD]     "
               "--- show help message of CMD" << std::endl;
  std::cout << "  quit/q              "
               "--- quit program" << std::endl;
  std::cout << "  break/b   [ADDR]    "
               "--- set breakpoint at ADDR" << std::endl;
  std::cout << "  watch/w   EXPR      "
               "--- set watchpoint at EXPR" << std::endl;
  std::cout << "  delete/d  [N]       "
               "--- delete breakpoint/watchpoint" << std::endl;
  std::cout << "  continue/c          "
               "--- continue running" << std::endl;
  std::cout << "  stepi/si  [N]       "
               "--- step by N instructions" << std::endl;
  std::cout << "  print/p   [EXPR]    "
               "--- show value of EXPR" << std::endl;
  std::cout << "  x         N EXPR    "
               "--- examine memory at EXPR" << std::endl;
  std::cout << "  disasm/da [N EXPR]  "
               "--- disassemble memory at EXPR" << std::endl;
  std::cout << "  info      ITEM      "
               "--- show information of ITEM" << std::endl;
}

// display help message of command
void PrintHelp(CommandName cmd) {
  switch (cmd) {
    case CommandName::Help: {
      std::cout << "Syntax: help [CMD]" << std::endl;
      std::cout << "  Show a list of all debugger commands, or give "
                   "details about a specific command." << std::endl;
      break;
    }
    case CommandName::Quit: {
      std::cout << "Syntax: quit/q" << std::endl;
      std::cout << "  Quit Risky32 and debugger." << std::endl;
      break;
    }
    case CommandName::Break: {
      std::cout << "Syntax: break/b [ADDR]" << std::endl;
      std::cout << "  Set a breakpoint at specific address (PC), "
                   "ADDR defaults to current PC." << std::endl;
      break;
    }
    case CommandName::Watch: {
      std::cout << "Syntax: watch/w EXPR" << std::endl;
      std::cout << "  Set a watchpoint for a specific expression, "
                   "pause when EXPR changes." << std::endl;
      break;
    }
    case CommandName::Delete: {
      std::cout << "Syntax: delete/d [N]" << std::endl;
      std::cout << "  Delete breakpoint/watchpoint N, delete all "
                   "breakpoints and watchpoints by default." << std::endl;
      break;
    }
    case CommandName::Continue: {
      std::cout << "Syntax: continue/c" << std::endl;
      std::cout << "  Continue running current program." << std::endl;
      break;
    }
    case CommandName::StepInst: {
      std::cout << "Syntax: stepi/si [N]" << std::endl;
      std::cout << "  Step by N instructions, "
                   "N defaults to 1." << std::endl;
      break;
    }
    case CommandName::Print: {
      std::cout << "Syntax: print/p [EXPR]" << std::endl;
      std::cout << "  Show value of EXPR, "
                   "or just show last value." << std::endl;
      break;
    }
    case CommandName::Examine: {
      std::cout << "Syntax: x N EXPR" << std::endl;
      std::cout << "  Examine N units memory at address EXPR, "
                   "4 bytes per unit." << std::endl;
      break;
    }
    case CommandName::Disasm: {
      std::cout << "Syntax: disasm/da [N EXPR]" << std::endl;
      std::cout << "  Disassemble N units memory at address EXPR, "
                   "4 bytes per unit. EXPR must be aligned." << std::endl;
      std::cout << "  Display 10 instructions near current PC "
                   "by default." << std::endl;
      break;
    }
    case CommandName::Info: {
      std::cout << "Syntax: info ITEM" << std::endl;
      std::cout << "  Show information of ITEM.\n" << std::endl;
      std::cout << "ITEM:" << std::endl;
      std::cout << "  reg/r   --- registers" << std::endl;
      std::cout << "  csr/c   --- CSRs" << std::endl;
      std::cout << "  break/b --- breakpoints" << std::endl;
      std::cout << "  watch/w --- watchpoints" << std::endl;
      break;
    }
    default: {
      LogError("unknown command, try 'help' to see command list");
      break;
    }
  }
}

}  // namespace

// definition of user's pause flag, default to 'true'
// because the debugger interface must be entered after startup
volatile sig_atomic_t Debugger::user_pause_ = 1;

void Debugger::SignalHandler(int sig) {
  assert(sig == SIGINT);
  user_pause_ = 1;
}

void Debugger::InitSignal() {
  // initialize sigaction
  sig_ = std::make_unique<struct sigaction>();
  sig_->sa_handler = SignalHandler;
  sigemptyset(&sig_->sa_mask);
  sig_->sa_flags = 0;
  // register sigaction
  sigaction(SIGINT, sig_.get(), nullptr);
  // show message
  std::cout << "Debugger is ready, try 'help' to see command list."
            << std::endl;
}

bool Debugger::CheckWatchpoints() {
  for (auto &&it : watches_) {
    // get watchpoint info
    auto &info = it.second;
    // evaluate new value
    std::uint32_t cur_val;
    auto ret = expr_eval_.Eval(info.record_id, cur_val);
    assert(ret);
    if (cur_val != info.last_val) {
      // record change
      std::cout << "watchpoint " << it.first << " hit ($"
                << info.record_id << ")" << std::endl;
      std::cout << "  old value: " << info.last_val << std::endl;
      std::cout << "  new value: " << cur_val << std::endl;
      // update watchpoint info
      info.last_val = cur_val;
      ++info.hit_count;
      return true;
    }
  }
  return false;
}

bool Debugger::Eval(std::string_view expr, std::uint32_t &ans) {
  return Eval(expr, ans, true);
}

bool Debugger::Eval(std::string_view expr, std::uint32_t &ans,
                    bool record) {
  auto ret = expr_eval_.Eval(expr, ans, record);
  if (!ret) LogError("invalid exprssion");
  return ret;
}

bool Debugger::DeleteBreak(std::uint32_t id) {
  // try to find breakpoint info
  auto it = breaks_.find(id);
  if (it == breaks_.end()) return false;
  const auto &info = it->second;
  // delete breakpoint
  core_.raw_bus()->WriteWord(info.addr, info.org_inst);
  if (cur_bp_ == &info) {
    core_.ReExecute(info.org_inst);
    cur_bp_ = nullptr;
  }
  pc_bp_.erase(info.addr);
  breaks_.erase(it);
  return true;
}

bool Debugger::DeleteWatch(std::uint32_t id) {
  // try to find watchpoint info
  auto it = watches_.find(id);
  if (it == watches_.end()) return false;
  const auto &info = it->second;
  // delete watchpoint
  expr_eval_.RemoveRecord(info.record_id);
  watches_.erase(it);
  return true;
}

void Debugger::ShowDisasm() {
  auto base = core_.pc() - 2 * 4;
  ShowDisasm(base, 10);
}

void Debugger::ShowDisasm(std::uint32_t base, std::uint32_t count) {
  assert((base & 0b11) == 0 && count);
  // get all disassembly
  std::vector<DisasmInfo> code;
  int padding = 0;
  bool inc_bp = false;
  for (std::uint32_t i = 0; i < count; ++i) {
    auto addr = base + i * 4;
    // get instruction data
    std::uint32_t inst_data;
    bool is_bp = false;
    for (const auto &it : breaks_) {
      if (it.second.addr == addr) {
        inst_data = it.second.org_inst;
        is_bp = true;
        break;
      }
    }
    if (!is_bp) inst_data = core_.raw_bus()->ReadWord(addr);
    // get disassembly
    auto disasm = Disassemble(inst_data, addr);
    code.push_back({is_bp, addr, inst_data, disasm});
    // update padding width & breakpoint flag
    if (disasm.first.size() > padding) padding = disasm.first.size();
    if (!inc_bp && is_bp) inc_bp = is_bp;
  }
  // print disassembly
  auto cur_pc = cur_bp_ ? cur_bp_->addr : core_.pc();
  for (const auto &i : code) {
    // print breakpoint info
    if (inc_bp) {
      if (i.is_breakpoint) {
        std::cout << style("D") << " B> ";
      }
      else {
        std::cout << "    ";
      }
    }
    // print current address
    if (i.addr == cur_pc) {
      std::cout << style("I") << std::hex << std::setw(8)
                << std::setfill('0') << std::right << i.addr << style("R")
                << ":  ";
    }
    else {
      std::cout << std::hex << std::setw(8) << std::setfill('0')
                << std::right << i.addr << ":  ";
    }
    // print raw instruction data
    std::cout << std::hex << std::setw(8) << std::setfill('0') << std::right
              << i.inst_data << std::dec << "      ";
    // print disassembly
    std::cout << style("B") << i.disasm.first;
    std::cout << std::setw(padding + 2 - i.disasm.first.size())
              << std::setfill(' ') << ' ';
    std::cout << i.disasm.second << std::endl;
  }
}

void Debugger::AcceptCommand() {
  // print disassembly when step/breakpoint
  if (!step_count_ || dbg_pause_) {
    std::cout << std::endl;
    ShowDisasm();
  }
  // clear debugger state
  user_pause_ = 0;
  step_count_ = -1;
  dbg_pause_ = false;
  // enter command line interface
  bool quit = false;
  std::istringstream iss;
  while (!quit) {
    std::cout << std::endl;
    // read line from terminal
    auto line = readline(prompt_.data());
    if (!line) {
      // EOF, just exit
      std::cout << "quit" << std::endl;
      std::exit(0);
    }
    if (*line) {
      // add current line to history
      add_history(line);
      // reset string stream
      iss.str(line);
      iss.clear();
      // parse command
      quit = ParseCommand(iss);
    }
    // free line buffer
    std::free(line);
  }
}

bool Debugger::ParseCommand(std::istream &is) {
  std::string cmd;
  // read command
  is >> cmd;
  if (cmd.empty()) return false;
  // execute command
  switch (GetCommandName(cmd)) {
    case CommandName::Help: {
      if (is.eof()) {
        PrintHelp();
      }
      else {
        is >> cmd;
        PrintHelp(GetCommandName(cmd));
      }
      break;
    }
    case CommandName::Quit: {
      std::exit(0);
      break;
    }
    case CommandName::Break: {
      CreateBreak(is);
      break;
    }
    case CommandName::Watch: {
      CreateWatch(is);
      break;
    }
    case CommandName::Delete: {
      DeletePoint(is);
      break;
    }
    case CommandName::Continue: {
      return true;
    }
    case CommandName::StepInst: {
      return StepByInst(is);
    }
    case CommandName::Print: {
      PrintExpr(is);
      break;
    }
    case CommandName::Examine: {
      ExamineMem(is);
      break;
    }
    case CommandName::Disasm: {
      DisasmMem(is);
      break;
    }
    case CommandName::Info: {
      PrintInfo(is);
      break;
    }
    default: {
      LogError("unknown command, try 'help' to see command list");
      break;
    }
  }
  return false;
}

void Debugger::CreateBreak(std::istream &is) {
  // get address of breakpoint
  std::string expr;
  std::uint32_t addr;
  if (is.eof()) {
    addr = core_.pc();
  }
  else {
    if (!std::getline(is, expr) || !Eval(expr, addr, false)) {
      LogError("invalid address");
      return;
    }
  }
  if (addr & 0b11) {
    LogError("address misaligned, invalid breakpoint");
    return;
  }
  // check for duplicates
  if (pc_bp_.find(addr) != pc_bp_.end()) {
    LogError("there is already a breakpoint at specific address");
    return;
  }
  // replace original instruction
  auto org_inst = core_.raw_bus()->ReadWord(addr);
  core_.raw_bus()->WriteWord(addr, kBreakInst);
  // store breakpoint info
  auto ret = breaks_.insert({next_id_++, {addr, org_inst, 0}});
  assert(ret.second);
  pc_bp_.insert({addr, &ret.first->second});
}

void Debugger::CreateWatch(std::istream &is) {
  // get expression
  std::string expr;
  if (!std::getline(is, expr)) {
    LogError("invalid 'EXPR', try 'help watch'");
    return;
  }
  // evaluate and record expression
  std::uint32_t value, id = expr_eval_.next_id();
  if (!Eval(expr, value)) return;
  // store watchpoint info
  watches_.insert({next_id_++, {id, value, 0}});
}

void Debugger::DeletePoint(std::istream &is) {
  if (is.eof()) {
    // show confirm message
    std::cout << "are you sure to delete all "
                 "breakpoints & watchpoints? [y/n] ";
    std::string line;
    if (!std::getline(std::cin, line) || line.size() != 1) return;
    if (std::tolower(line[0]) != 'y') return;
    // delete all breakpoints
    auto breaks = breaks_;
    for (const auto &i : breaks) DeleteBreak(i.first);
    // delete all watchpoints
    auto watches = watches_;
    for (const auto &i : watches) DeleteWatch(i.first);
  }
  else {
    // get id from input
    std::uint32_t n;
    is >> n;
    if (!is) {
      LogError("invalid breakpoint/watchpoint id");
      return;
    }
    // delete point by id
    if (!DeleteBreak(n) && !DeleteWatch(n)) {
      LogError("breakpoint/watchpoint not found");
    }
  }
}

bool Debugger::StepByInst(std::istream &is) {
  // set step count
  if (is.eof()) {
    step_count_ = 1;
  }
  else {
    is >> step_count_;
    if (!is || step_count_ <= 0) {
      LogError("invalid step count");
      step_count_ = -1;
      return false;
    }
  }
  // return from debugger
  return true;
}

void Debugger::PrintExpr(std::istream &is) {
  std::uint32_t value, id;
  // get expression
  std::string expr;
  if (!std::getline(is, expr)) {
    // show last value
    if (!expr_eval_.next_id()) {
      LogError("there is no last value available");
      return;
    }
    else {
      id = expr_eval_.next_id() - 1;
      auto ret = expr_eval_.Eval(id, value);
      assert(ret);
    }
  }
  else {
    // evaluate expression
    id = expr_eval_.next_id();
    if (!Eval(expr, value)) return;
  }
  // print result
  std::cout << "$" << id << " = " << value << std::endl;
}

void Debugger::ExamineMem(std::istream &is) {
  // get parameters
  std::uint32_t addr, n;
  std::string expr;
  is >> n;
  if (!is || !n) {
    LogError("invalid count 'N', try 'help x'");
    return;
  }
  if (!std::getline(is, expr)) {
    LogError("invalid 'EXPR', try 'help x'");
    return;
  }
  if (!Eval(expr, addr, false)) return;
  // print memory units
  while (n--) {
    std::cout << std::hex << std::setfill('0') << std::setw(8) << addr;
    std::cout << ": " << std::setw(2) << std::setfill('0')
              << static_cast<int>(core_.raw_bus()->ReadByte(addr++)) << ' ';
    std::cout << std::setw(2) << std::setfill('0')
              << static_cast<int>(core_.raw_bus()->ReadByte(addr++)) << ' ';
    std::cout << std::setw(2) << std::setfill('0')
              << static_cast<int>(core_.raw_bus()->ReadByte(addr++)) << ' ';
    std::cout << std::setw(2) << std::setfill('0')
              << static_cast<int>(core_.raw_bus()->ReadByte(addr++));
    std::cout << std::dec << std::endl;
  }
}

void Debugger::DisasmMem(std::istream &is) {
  // no parameters
  if (is.eof()) {
    ShowDisasm();
    return;
  }
  // get parameters
  std::uint32_t base, n;
  std::string expr;
  is >> n;
  if (!is || !n) {
    LogError("invalid count 'N', try 'help disasm'");
    return;
  }
  if (!std::getline(is, expr)) {
    LogError("invalid 'EXPR', try 'help disasm'");
    return;
  }
  if (!Eval(expr, base, false)) return;
  if (base & 0b11) {
    LogError("'EXPR' is misaligned, try 'help disasm'");
    return;
  }
  // show disassembly
  ShowDisasm(base, n);
}

void Debugger::PrintInfo(std::istream &is) {
  // get item name
  std::string item;
  is >> item;
  auto it = kInfoItemMap.find(item);
  if (it == kInfoItemMap.end()) {
    LogError("invalid 'ITEM', try 'help info'");
    return;
  }
  // show information
  switch (it->second) {
    case InfoItem::Reg: {
      // register info
      expr_eval_.PrintRegInfo(std::cout);
      break;
    }
    case InfoItem::CSR: {
      // CSR info
      expr_eval_.PrintCSRInfo(std::cout);
      break;
    }
    case InfoItem::Break: {
      // breakpoint info
      if (breaks_.empty()) {
        std::cout << "no breakpoints currently set" << std::endl;
      }
      else {
        std::cout << "number of breakpoints: " << breaks_.size()
                  << std::endl;
        for (const auto &it : breaks_) {
          const auto &info = it.second;
          std::cout << "  breakpoint #" << it.first << ": pc = 0x"
                    << std::hex << std::setw(8) << std::setfill('0')
                    << info.addr << std::dec << ", hit_count = "
                    << info.hit_count << std::endl;
        }
      }
      break;
    }
    case InfoItem::Watch: {
      // watchpoint info
      if (watches_.empty()) {
        std::cout << "no watchpoints currently set" << std::endl;
      }
      else {
        std::cout << "number of watchpoints: " << watches_.size()
                  << std::endl;
        for (const auto &it : watches_) {
          const auto &info = it.second;
          std::cout << "  watchpoint #" << it.first << ": $"
                    << info.record_id << " = '";
          expr_eval_.PrintExpr(std::cout, info.record_id);
          std::cout << "', value = " << info.last_val
                    << ", hit_count = " << info.hit_count << std::endl;
        }
      }
      break;
    }
    default: assert(false);
  }
}

std::uint8_t Debugger::ReadByte(std::uint32_t addr) {
  return 0;
}

void Debugger::WriteByte(std::uint32_t addr, std::uint8_t value) {
  // do nothing
}

std::uint16_t Debugger::ReadHalf(std::uint32_t addr) {
  return 0;
}

void Debugger::WriteHalf(std::uint32_t addr, std::uint16_t value) {
  // do nothing
}

std::uint32_t Debugger::ReadWord(std::uint32_t addr) {
  return 0;
}

void Debugger::WriteWord(std::uint32_t addr, std::uint32_t value) {
  if (addr == kAddrBreak) {
    // breakpoint triggered
    dbg_pause_ = true;
    // update current breakpoint info
    auto it = pc_bp_.find(core_.pc());
    assert(it != pc_bp_.end());
    cur_bp_ = it->second;
    // update hit count
    ++it->second->hit_count;
    // show message
    std::cout << "breakpoint hit, pc = 0x" << std::hex << std::setw(8)
              << std::setfill('0') << it->first << std::dec << std::endl;
  }
}

void Debugger::NextCycle() {
  // check user interrupt or breakpoints
  if (user_pause_ || dbg_pause_) AcceptCommand();
  // check watchpoints
  if (!watches_.empty() && CheckWatchpoints()) AcceptCommand();
  // check/update step count
  if (!step_count_) AcceptCommand();
  if (step_count_ > 0) --step_count_;
  // run next core cycle
  if (cur_bp_) {
    core_.ReExecute(cur_bp_->org_inst);
    cur_bp_ = nullptr;
  }
  else {
    core_.NextCycle();
  }
}
