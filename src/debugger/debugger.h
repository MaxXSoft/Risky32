#ifndef RISKY32_DEBUGGER_DEBUGGER_H_
#define RISKY32_DEBUGGER_DEBUGGER_H_

#include <memory>
#include <string_view>
#include <istream>
#include <cstdint>

#include <signal.h>

#include "core/core.h"
#include "peripheral/storage/rom.h"
#include "peripheral/peripheral.h"
#include "debugger/expreval.h"

class Debugger : public PeripheralInterface {
 public:
  Debugger(Core &core, ROM &rom)
      : core_(core), rom_(rom), prompt_("risky32> "),
        step_count_(0), expr_eval_(core) {
    InitSignal();
  }

  std::uint8_t ReadByte(std::uint32_t addr) override;
  void WriteByte(std::uint32_t addr, std::uint8_t value) override;
  std::uint16_t ReadHalf(std::uint32_t addr) override;
  void WriteHalf(std::uint32_t addr, std::uint16_t value) override;
  std::uint32_t ReadWord(std::uint32_t addr) override;
  void WriteWord(std::uint32_t addr, std::uint32_t value) override;
  std::uint32_t size() const override { return 16; }

  // accept user input
  void AcceptCommand();

  // setters
  void set_prompt(std::string_view prompt) { prompt_ = prompt; }

  // getters
  bool pause() const { return pause_; }

 private:
  // signal handler (C-c)
  static void SignalHandler(int sig);
  // initialize signal handler
  void InitSignal();

  // parse command, returns true if need to return from debugger
  bool ParseCommand(std::istream &is);
  // evaluate expression with record
  bool Eval(std::string_view expr, std::uint32_t &ans);
  // evaluate expression
  bool Eval(std::string_view expr, std::uint32_t &ans, bool record);
  // print information ('info ITEM' command)
  void PrintInfo(std::istream &is);
  // examine memory ('x EXPR [N]' command)
  void ExamineMem(std::istream &is);

  // pause flag when pressed C-c
  static volatile sig_atomic_t pause_;
  // pointer of sigaction
  std::unique_ptr<struct sigaction> sig_;
  // reference of emulation core
  Core &core_;
  // reference of ROM
  ROM &rom_;
  // prompt of debugger
  std::string_view prompt_;
  // step count
  std::uint32_t step_count_;
  // evaluator
  ExprEvaluator expr_eval_;
};

#endif  // RISKY32_DEBUGGER_DEBUGGER_H_
