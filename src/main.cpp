#include <iostream>
#include <cstring>
#include <cstddef>

#include "core/core.h"
#include "bus/bus.h"
#include "peripheral/general/gpio.h"
#include "peripheral/interrupt/clint.h"
#include "peripheral/storage/ram.h"
#include "peripheral/storage/rom.h"

#include "define/mmio.h"
#include "version.h"

using namespace std;

namespace {

void PrintVersion() {
  cout << APP_NAME << " version " << APP_VERSION << endl;
  cout << "A simple RISC-V simulator written in C++." << endl;
  cout << endl;
  cout << "Copyright (C) 2010-2019 MaxXing, MaxXSoft. License GPLv3.";
  cout << endl;
}

}  // namespace

int main(int argc, const char *argv[]) {
  // check argument
  if (argc < 2) {
    cerr << "error: invalid argument" << endl;
    cerr << "usage: " << argv[0] << " [-v] binary" << endl;
    return 1;
  }
  else if (!strcmp(argv[1], "-v")) {
    PrintVersion();
    return 0;
  }

  // create peripherals
  auto rom = std::make_shared<ROM>();
  auto ram = std::make_shared<RAM>(65536);
  auto gpio = std::make_shared<GPIO>();
  auto clint = std::make_shared<CLINT>();
  if (!rom->LoadBinary(argv[1])) {
    cerr << "error: failed to load file '" << argv[1] << "'" << endl;
    return 1;
  }

  // initialize system bus
  auto bus = std::make_shared<Bus>();
  bus->AddPeripheral(kMMIOAddrROM, rom);
  bus->AddPeripheral(kMMIOAddrRAM, ram);
  bus->AddPeripheral(kMMIOAddrGPIO, gpio);
  bus->AddPeripheral(kMMIOAddrCLINT, clint);

  // initialize core
  Core core(bus);
  core.set_timer_int(clint->timer_int());
  core.set_soft_int(clint->soft_int());
  core.Reset();

  // run simulation
  while (!gpio->halt()) {
    core.NextCycle();
    clint->UpdateTimer();
  }
  return 0;
}
