#include <iostream>
#include <string>
#include <string_view>
#include <cctype>
#include <cstddef>

#include "core/core.h"
#include "bus/bus.h"
#include "peripheral/general/gpio.h"
#include "peripheral/interrupt/clint.h"
#include "peripheral/storage/ram.h"
#include "peripheral/storage/rom.h"
#include "peripheral/general/confreg.h"
#include "debugger/debugger.h"

#include "define/mmio.h"
#include "util/argparse.h"
#include "version.h"

using namespace std;

namespace {

// print version info to stdout
void PrintVersion() {
  cout << APP_NAME << " version " << APP_VERSION << endl;
  cout << "A simple RISC-V emulator written in C++." << endl;
  cout << endl;
  cout << "Copyright (C) 2010-2019 MaxXing, MaxXSoft. License GPLv3.";
  cout << endl;
}

// convert string to memory size, returns 0 if error
size_t GetMemSize(string_view size_str) {
  int scale = 0;
  if (isalpha(size_str.back())) {
    char unit = size_str.back();
    size_str = size_str.substr(0, size_str.size() - 1);
    if (tolower(unit) == 'k') {
      scale = 1024;
    }
    else if (tolower(unit) == 'm') {
      scale = 1024 * 1024;
    }
  }
  else {
    scale = 1;
  }
  return stoi({size_str.data(), size_str.size()}) * scale;
}

}  // namespace

int main(int argc, const char *argv[]) {
  // set up argument parser
  ArgParser argp;
  argp.AddArgument<string>("binary", "input binary file");
  argp.AddOption<bool>("help", "h", "show this message", false);
  argp.AddOption<bool>("version", "v", "show version info", false);
  argp.AddOption<bool>("debug", "d", "enable built-in debugger", false);
  argp.AddOption<string>("mem", "m", "set memory size (default to '64k')",
                         "64k");
  argp.AddOption<string>("flash", "f", "load another binary file to flash",
                         "");

  // parse argument
  auto ret = argp.Parse(argc, argv);

  // check if need to exit program
  if (argp.GetValue<bool>("help")) {
    argp.PrintHelp();
    return 0;
  }
  else if (argp.GetValue<bool>("version")) {
    PrintVersion();
    return 0;
  }
  else if (!ret) {
    cerr << "invalid input, run '";
    cerr << argp.program_name() << " -h' for help" << endl;
    return 1;
  }

  // get input
  size_t mem_size = GetMemSize(argp.GetValue<string>("mem"));
  auto file = argp.GetValue<string>("binary");
  auto flash_file = argp.GetValue<string>("flash");
  if (!mem_size || (mem_size & 0b11)) {
    cerr << "error: invalid memory size (" << mem_size << ')' << endl;
    return 1;
  }

  // create peripherals
  auto rom = make_shared<ROM>();
  auto ram = make_shared<RAM>(mem_size);
  auto gpio = make_shared<GPIO>();
  auto clint = make_shared<CLINT>();
  auto flash = make_shared<ROM>();
  auto confreg = make_shared<ConfReg>(1);
  if (!rom->LoadBinary(file)) {
    cerr << "error: failed to load file '" << file << "'" << endl;
    return 1;
  }
  if (!flash_file.empty() && !flash->LoadBinary(flash_file)) {
    cerr << "error: failed to load file '" << flash_file << "'" << endl;
    return 1;
  }

  // initialize system bus
  auto bus = make_shared<Bus>();
  bus->AddPeripheral(kMMIOAddrROM, rom);
  bus->AddPeripheral(kMMIOAddrRAM, ram);
  bus->AddPeripheral(kMMIOAddrGPIO, gpio);
  bus->AddPeripheral(kMMIOAddrCLINT, clint);
  bus->AddPeripheral(kMMIOAddrConfReg, confreg);
  if (flash->size()) {
    bus->AddPeripheral(kMMIOAddrFlash, flash);
  }

  // initialize core
  Core core(bus);
  core.set_timer_int(clint->timer_int());
  core.set_soft_int(clint->soft_int());
  core.Reset();

  if (argp.GetValue<bool>("debug")) {
    PrintVersion();
    cout << endl;
    // run debugger
    auto debugger = make_shared<Debugger>(core);
    bus->AddPeripheral(kMMIOAddrDebugger, debugger);
    while (!gpio->halt()) {
      clint->UpdateTimer();
      debugger->NextCycle();
    }
  }
  else {
    // run emulation
    while (!gpio->halt()) {
      clint->UpdateTimer();
      core.NextCycle();
    }
  }

  // return the value of register 'a0' as exit code
  return core.regs(10);
}
