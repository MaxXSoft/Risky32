#include "debugger/debugger.h"

#include <unordered_map>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdlib>

#include "readline/readline.h"
#include "readline/history.h"

namespace {

// MMIO address of 'break' operation
constexpr std::uint32_t kAddrBreak = 0x0;

// name of all debugger commands
enum class CommandName {
  Unknown,
  Help, Quit,
  Run, Break, Watch, Delete,
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
  {"run", CommandName::Run}, {"r", CommandName::Run},
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

// display help message
void PrintHelp() {
  std::cout << "Debugger commands:" << std::endl;
  std::cout << "  help      [CMD]     "
               "--- show help message of CMD" << std::endl;
  std::cout << "  quit/q              "
               "--- quit program" << std::endl;
  std::cout << "  run/r     [FILE]    "
               "--- run FILE" << std::endl;
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
  std::cout << "  x         EXPR [N]  "
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
    case CommandName::Run: {
      std::cout << "Syntax: run/r [FILE]" << std::endl;
      std::cout << "  Run current program, "
                   "or run a specific file" << std::endl;
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
      std::cout << "Syntax: x EXPR [N]" << std::endl;
      std::cout << "  Examine N units memory at address EXPR, "
                   "N defaults to 1." << std::endl;
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
      std::cout << "ERROR: unknown command, "
                   "try 'help' to see command list." << std::endl;
      break;
    }
  }
}

}  // namespace

volatile sig_atomic_t Debugger::pause_ = 0;

void Debugger::SignalHandler(int sig) {
  assert(sig == SIGINT);
  pause_ = 1;
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
    case CommandName::Run: {
      // TODO
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
      //
      break;
    }
    case CommandName::Continue: {
      pause_ = 0;
      return true;
    }
    case CommandName::StepInst: {
      // set step count
      if (is.eof()) {
        step_count_ = 1;
      }
      else {
        is >> step_count_;
        if (!is) {
          std::cout << "ERROR: invalid step count" << std::endl;
          return false;
        }
      }
      // return from debugger
      return true;
    }
    case CommandName::Print: {
      // TODO: expression
      break;
    }
    case CommandName::Examine: {
      // TODO: expression
      break;
    }
    case CommandName::Info: {
      PrintInfo(is);
      break;
    }
    default: {
      std::cout << "ERROR: invalid command '" << cmd << "'" << std::endl;
      break;
    }
  }
  return false;
}

void Debugger::PrintInfo(std::istream &is) {
  // get item name
  std::string item;
  is >> item;
  auto it = kInfoItemMap.find(item);
  if (it == kInfoItemMap.end()) {
    std::cout << "ERROR: invalid 'ITEM', try 'help info'" << std::endl;
    return;
  }
  // show information
  switch (it->second) {
    case InfoItem::Reg: {
      //
      break;
    }
    case InfoItem::CSR: {
      //
      break;
    }
    case InfoItem::Break: {
      //
      break;
    }
    case InfoItem::Watch: {
      //
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
    pause_ = 1;
  }
}

void Debugger::AcceptCommand() {
  // check if is not stepping
  if (step_count_ > 0) {
    --step_count_;
    return;
  }
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
