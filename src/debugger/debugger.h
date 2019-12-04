#ifndef RISKY32_DEBUGGER_DEBUGGER_H_
#define RISKY32_DEBUGGER_DEBUGGER_H_

#include <memory>
#include <string_view>
#include <istream>
#include <unordered_map>
#include <cstdint>

#include <signal.h>

#include "core/core.h"
#include "peripheral/storage/rom.h"
#include "peripheral/peripheral.h"
#include "debugger/expreval.h"

class Debugger : public PeripheralInterface {
 public:
  Debugger(Core &core, ROM &rom)
      : core_(core), rom_(rom), expr_eval_(core), prompt_("risky32> "),
        dbg_pause_(false), step_count_(0), next_id_(0) {
    InitSignal();
  }

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;
  std::uint32_t size() const override { return 16; }

  // emulate next cycle
  void NextCycle();

  // setters
  void set_prompt(std::string_view prompt) { prompt_ = prompt; }

 private:
  // breakpoint information
  struct BreakInfo {
    // PC address of breakpoint
    std::uint32_t addr;
    // original instruction at PC address
    std::uint32_t org_inst;
  };

  // watchpoint information
  struct WatchInfo {
    // expression record id (in 'ExprEvaluator')
    std::uint32_t record_id;
    // last value
    std::uint32_t last_val;
  };

  // signal handler (C-c)
  static void SignalHandler(int sig);
  // initialize signal handler
  void InitSignal();
  // check if there are any watchpoints hit
  bool CheckWatchpoints();
  // evaluate expression with record
  bool Eval(std::string_view expr, std::uint32_t &ans);
  // evaluate expression
  bool Eval(std::string_view expr, std::uint32_t &ans, bool record);

  // accept user input
  void AcceptCommand();
  // parse command, returns true if need to return from debugger
  bool ParseCommand(std::istream &is);
  // delete breakpoint/watchpoint ('delete [N]' command)
  void DeletePoint(std::istream &is);
  // step by machine instructions ('stepi [N]' command)
  bool StepByInst(std::istream &is);
  // print value of expression ('print EXPR' command)
  void PrintExpr(std::istream &is);
  // examine memory ('x N EXPR' command)
  void ExamineMem(std::istream &is);
  // print information ('info ITEM' command)
  void PrintInfo(std::istream &is);

  // pause flag when pressed C-c, or need to step/watch
  static volatile sig_atomic_t user_pause_;
  // pointer of sigaction
  std::unique_ptr<struct sigaction> sig_;

  // reference of emulation core
  Core &core_;
  // reference of ROM
  ROM &rom_;

  // evaluator
  ExprEvaluator expr_eval_;
  // prompt of debugger
  std::string_view prompt_;
  // pause flag of debugger
  bool dbg_pause_;
  // step count
  int step_count_;
  // breakpoint list
  std::unordered_map<std::uint32_t, BreakInfo> breaks_;
  // watchpoint list
  std::unordered_map<std::uint32_t, WatchInfo> watches_;
  // next breakpoint/watchpoint id
  std::uint32_t next_id_;
};

#endif  // RISKY32_DEBUGGER_DEBUGGER_H_
