#include <iostream>
#include <string_view>
#include <cstring>
#include <cstddef>

#include "core/core.h"
#include "bus/bus.h"
#include "peripheral/general/gpio.h"
#include "peripheral/interrupt/clint.h"
#include "peripheral/storage/ram.h"
#include "peripheral/storage/rom.h"
#include "debugger/debugger.h"

#include "define/mmio.h"
#include "version.h"

using namespace std;

namespace {

void PrintVersion() {
  cout << APP_NAME << " version " << APP_VERSION << endl;
  cout << "A simple RISC-V emulator written in C++." << endl;
  cout << endl;
  cout << "Copyright (C) 2010-2019 MaxXing, MaxXSoft. License GPLv3.";
  cout << endl;
}

}  // namespace

int main(int argc, const char *argv[]) {
  bool debug = false;
  string_view file;

  // check argument
  if (argc < 2) {
    cerr << "error: invalid argument" << endl;
    cerr << "usage: " << argv[0] << " [-v|-d] binary" << endl;
    return 1;
  }
  else {
    // scan command line arguments
    for (int i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-v")) {
        PrintVersion();
        return 0;
      }
      else if (!strcmp(argv[i], "-d")) {
        debug = true;
      }
      else if (argv[i][0] != '-' && file.empty()) {
        file = argv[i];
      }
    }
  }

  // create peripherals
  auto rom = make_shared<ROM>();
  auto ram = make_shared<RAM>(65536);
  auto gpio = make_shared<GPIO>();
  auto clint = make_shared<CLINT>();
  if (!rom->LoadBinary(file)) {
    cerr << "error: failed to load file '" << file << "'" << endl;
    return 1;
  }

  // initialize system bus
  auto bus = make_shared<Bus>();
  bus->AddPeripheral(kMMIOAddrROM, rom);
  bus->AddPeripheral(kMMIOAddrRAM, ram);
  bus->AddPeripheral(kMMIOAddrGPIO, gpio);
  bus->AddPeripheral(kMMIOAddrCLINT, clint);

  // initialize core
  Core core(bus);
  core.set_timer_int(clint->timer_int());
  core.set_soft_int(clint->soft_int());
  core.Reset();

  if (debug) {
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
