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

    BusSubscriber(std::uint16_t startAddr, std::uint16_t endAddr) : startAddr(startAddr), endAddr(endAddr) {}

    virtual ~BusSubscriber() = default;

    virtual void writeHandler(std::uint16_t addr, std::uint8_t data) = 0;
    virtual std::uint8_t readHandler(std::uint16_t addr) = 0;

  private:
    std::uint16_t startAddr;
    std::uint16_t endAddr;
};

class Bus {
  public:
    Bus() = default;
    ~Bus() = default;

    void addDevice(BusSubscriber* device);

    void write(std::uint16_t addr, std::uint8_t data) const noexcept;

    [[nodiscard]] std::uint8_t read(std::uint16_t addr) const noexcept;

  private:
    std::vector<BusSubscriber*> devices;

    [[nodiscard]] BusSubscriber* findDevice(std::uint16_t addr) const noexcept;
};

}  // namespace NES

#endif  //NES_EMULATOR_BUS_HPP
