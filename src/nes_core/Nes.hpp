#ifndef NES_EMULATOR_NES_HPP
#define NES_EMULATOR_NES_HPP

#include <cstdint>
#include "comm/Bus.hpp"
#include "cpu/Mos6502.hpp"
#include "memory/Ram.hpp"

namespace NES {

// TODO: pending to add documentation

class NesSystem {
  public:
    NesSystem();
    ~NesSystem();

    void reset();
    void cycle();

  private:
    NES::Bus cpuBus;
    NES::Ram ram;
    NES::MOS6502 cpu;
};

}  // namespace NES

#endif  //NES_EMULATOR_NES_HPP
