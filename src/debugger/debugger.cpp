#include "debugger/debugger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <cctype>

#include "readline/readline.h"
#include "readline/history.h"

namespace {

// MMIO address of 'break' operation
constexpr std::uint32_t kAddrBreak = 0x0;

// name of all debugger commands
enum class CommandName {
  Unknown,
  Help, Quit,
  Break, Watch, Delete,
  Continue, StepInst,
  Print, Examine, Info
};

// name of all available ITEMs in command 'info'
enum class InfoItem {
  Reg, CSR, Break, Watch
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
  std::cout << "ERROR: " << msg << std::endl;
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

volatile sig_atomic_t Debugger::user_pause_ = 0;

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
      return true;
    }
  }
  return false;
}

bool Debugger::Eval(std::string_view expr, std::uint32_t &ans) {
  Eval(expr, ans, true);
}

bool Debugger::Eval(std::string_view expr, std::uint32_t &ans,
                    bool record) {
  auto ret = expr_eval_.Eval(expr, ans, record);
  if (!ret) LogError("invalid exprssion");
  return ret;
}

void Debugger::AcceptCommand() {
  // clear debugger state
  user_pause_ = 0;
  step_count_ = -1;
  dbg_pause_ = false;
  // enter command line interface
  bool quit = false;
  std::istringstream iss;
  while (!quit) {
    // read line from terminal
    auto line = readline(prompt_.data());
    if (!line) {
      // EOF, just exit
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
      //
      break;
    }
    case CommandName::Watch: {
      // TODO: expression
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

void Debugger::DeletePoint(std::istream &is) {
  std::uint32_t n;
  is >> n;
  if (!is) {
    // delete all breakpoints & watchpoints
    std::cout << "are you sure to delete all "
                 "breakpoints & watchpoints? [y/n]";
    if (std::tolower(std::cin.get()) != 'y') return;
    // TODO
  }
  else {
    // TODO
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
  // get expression
  std::string expr;
  if (!std::getline(is, expr)) {
    LogError("invalid 'EXPR', try 'help print'");
    return;
  }
  // evaluate expression
  std::uint32_t value, id = expr_eval_.next_id();
  if (!Eval(expr, value)) return;
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
    std::cout << std::hex << std::setfill('0');
    std::cout << std::setw(8) << addr << ": " << std::setw(2);
    std::cout << core_.raw_bus()->ReadByte(addr++) << ' ';
    std::cout << core_.raw_bus()->ReadByte(addr++) << ' ';
    std::cout << core_.raw_bus()->ReadByte(addr++) << ' ';
    std::cout << core_.raw_bus()->ReadByte(addr++);
    std::cout << std::dec << std::endl;
  }
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
          std::cout << "  breakpoint #" << it.first << ": pc = "
                    << std::hex << std::setw(8) << std::setfill('0')
                    << info.addr << std::dec << std::endl;
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
                    << info.record_id << " = (";
          expr_eval_.PrintExprInfo(std::cout, info.record_id);
          std::cout << "), value = " << info.last_val << std::endl;
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
    // trigger breakpoint
    dbg_pause_ = true;
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
  // next cycle for core
  core_.NextCycle();
}
