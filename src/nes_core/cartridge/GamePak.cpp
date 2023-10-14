#include "cartridge/GamePak.hpp"
#include "cartridge/mapper/Mapper_000.hpp"

namespace NES {

GamePak::GamePak(const iNES::iNES& rom) : prgRom(rom.prgRom), chrRom(rom.chrRom) {
    switch (rom.mapperNumber()) {
        case 0:
            mapper = std::make_unique<Mapper_000>(rom.header.nPrgRomBanks, rom.header.nChrRomBanks);
            break;
        default:
            assert(false && "Mapper not supported yet");
    }

    // TODO: configure the rest of the parameters
}

void GamePak::prgRomWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    std::uint32_t writeAddr = mapper->mapPrgRomWrite(addr);
    prgRom[writeAddr] = data;
}

std::uint8_t GamePak::prgRomRead(std::uint16_t addr) const noexcept {
    std::uint32_t readAddr = mapper->mapPrgRomRead(addr);
    return prgRom[readAddr];
}

void GamePak::chrRomWrite(std::uint16_t addr, std::uint8_t data) noexcept {
    std::uint32_t writeAddr = mapper->mapChrRomWrite(addr);
    chrRom[writeAddr] = data;
}

std::uint8_t GamePak::chrRomRead(std::uint16_t addr) const noexcept {
    std::uint32_t readAddr = mapper->mapChrRomRead(addr);
    return chrRom[readAddr];
}

Image<Palette, 8, 8> GamePak::getSprite(std::uint8_t tile, bool rightTable) const noexcept {
    /* Addressing of CHR ROM
     *
     * DCBA98 76543210
     * ---------------
     * 0HRRRR CCCCPTTT
     * |||||| |||||+++- T: Fine Y offset, the row number within a tile
     * |||||| ||||+---- P: Bit plane (0: "lower"; 1: "upper")
     * |||||| ++++----- C: Tile column
     * ||++++---------- R: Tile row
     * |+-------------- H: Half of pattern table (0: "left"; 1: "right")
     * +--------------- 0: Pattern table is at $0000-$1FFF
     */

    Image<Palette, 8, 8> sprite;
    constexpr std::size_t TILE_SIZE = 8;
    constexpr std::size_t TILE_BYTES = 16;

    // Calculate the address of the tile based on the table to use
    std::uint16_t baseAddr = rightTable == 0 ? 0x0000 : 0x1000;
    std::size_t tile_addr = baseAddr + tile * TILE_BYTES;

    for (size_t i = 0; i < TILE_SIZE; i++) {
        // Read a row of the tile
        size_t addr = tile_addr + i;
        std::uint8_t tileLsb = chrRomRead(static_cast<uint16_t>(addr));
        std::uint8_t tileMsb = chrRomRead(static_cast<uint16_t>(addr + TILE_BYTES / 2));

        for (size_t j = 0; j < TILE_SIZE; j++) {
            // Read a pixel of each row of the tile
            std::uint8_t lsb = static_cast<std::uint8_t>(tileLsb >> (7 - j)) & 0x01;
            std::uint8_t msb = static_cast<std::uint8_t>((tileMsb >> (7 - j)) << 1) & 0x02;
            sprite[i][j] = msb | lsb;
        }
    }

    return sprite;
}

}  // namespace NES