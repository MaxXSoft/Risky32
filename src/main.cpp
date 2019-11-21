#include "core/core.h"
#include "bus/bus.h"
#include "peripheral/general/gpio.h"
#include "peripheral/storage/ram.h"
#include "peripheral/storage/rom.h"

#include "define/exception.h"

using namespace std;

int main(int argc, const char *argv[]) {
  // check argument count
  if (argc < 2) return 1;

  // create peripherals
  auto rom = std::make_shared<ROM>();
  auto ram = std::make_shared<RAM>(65536);
  auto gpio = std::make_shared<GPIO>();
  if (!rom->LoadBinary(argv[1])) return 1;

  // initialize system bus
  Bus bus;
  bus.AddPeripheral(kResetVector, rom->size(), rom);
  bus.AddPeripheral(0x80000000, ram->size(), ram);
  bus.AddPeripheral(0x90000000, 512, gpio);

  // initialize core
  Core core(bus);
  core.Reset();

  // run simulation
  while (!gpio->halt()) core.NextCycle();
  return 0;
}
