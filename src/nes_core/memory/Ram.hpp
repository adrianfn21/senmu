#ifndef NES_EMULATOR_RAM_HPP
#define NES_EMULATOR_RAM_HPP

#include <cstdint>
#include <vector>
#include "comm/Bus.hpp"

namespace NES {

class Ram : private BusSubscriber {
  public:
    Ram(Bus& bus, uint16_t startAddr, uint16_t endAddr);

    ~Ram() override = default;

    void writeHandler(uint16_t addr, uint8_t data) override;

    [[nodiscard]] uint8_t readHandler(uint16_t addr) override;

  private:
    std::vector<uint8_t> memory;
};

}  // namespace NES

#endif  //NES_EMULATOR_RAM_HPP
