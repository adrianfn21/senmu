#ifndef NES_EMULATOR_BUS_HPP
#define NES_EMULATOR_BUS_HPP

#include <cassert>
#include <cstdint>
#include <vector>

namespace NES {

// TODO: pending to add documentation

class BusSubscriber {
  public:
    friend class Bus;

    BusSubscriber(uint16_t startAddr, uint16_t endAddr) : startAddr(startAddr), endAddr(endAddr) {}

    virtual ~BusSubscriber() = default;

    virtual void writeHandler(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t readHandler(uint16_t addr) = 0;

  private:
    uint16_t startAddr;
    uint16_t endAddr;
};

class Bus {
  public:
    Bus() = default;
    ~Bus() = default;

    void addDevice(BusSubscriber* device);

    void write(uint16_t addr, uint8_t data) const noexcept;

    [[nodiscard]] uint8_t read(uint16_t addr) const noexcept;

  private:
    std::vector<BusSubscriber*> devices;

    [[nodiscard]] BusSubscriber* findDevice(uint16_t addr) const noexcept;
};

}  // namespace NES

#endif  //NES_EMULATOR_BUS_HPP
