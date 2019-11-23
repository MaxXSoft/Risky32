#include <iostream>
#include <cstring>
#include <cstddef>

#include "core/core.h"
#include "bus/bus.h"
#include "peripheral/general/gpio.h"
#include "peripheral/storage/ram.h"
#include "peripheral/storage/rom.h"

#include "define/exception.h"
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

inline std::size_t RoundToPow2(std::size_t val) {
  auto v = val - 1;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  return v + 1;
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
  if (!rom->LoadBinary(argv[1])) {
    cerr << "error: failed to load file '" << argv[1] << "'" << endl;
    return 1;
  }

  // initialize system bus
  Bus bus;
  bus.AddPeripheral(kResetVector, RoundToPow2(rom->size()), rom);
  bus.AddPeripheral(0x80000000, RoundToPow2(ram->size()), ram);
  bus.AddPeripheral(0x90000000, 512, gpio);

  // initialize core
  Core core(bus);
  core.Reset();

  // run simulation
  while (!gpio->halt()) core.NextCycle();
  return 0;
}
