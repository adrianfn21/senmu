#include "ines/iNes.hpp"
#include <fstream>
#include <iostream>

namespace iNES {

iNES::iNES(const std::string& filepath) {
    // Open file
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Could not open file " << filepath << std::endl;
        exit(1);
    }

    std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check if file is iNES format
    if (data[0] != 0x4E || data[1] != 0x45 || data[2] != 0x53 || data[3] != 0x1A) {
        std::cerr << "File " << filepath << " is not in iNES format" << std::endl;
        exit(1);
    }

    // Read header
    header.prgRomSize = data[4] * PRG_ROM_SIZE;
    header.chrRomSize = data[5] * CHR_ROM_SIZE;
    header.flags6 = data[6];
    header.flags7 = data[7];
    header.flags8 = data[8];
    header.flags9 = data[9];
    header.flags10 = data[10];
    header.playchoiceInstRomSize = 0 * PLAYCHOICE_INST_ROM_SIZE;
    header.playchoicePromSize = 0 * PLAYCHOICE_PROM_SIZE;

    // Read PRG ROM
    const size_t START_PRG_ROM = HEADER_SIZE;
    prgRom.resize(header.prgRomSize);
    std::copy(data.begin() + START_PRG_ROM, data.begin() + START_PRG_ROM + header.prgRomSize, std::back_inserter(prgRom));

    // Read CHR ROM
    const size_t START_CHR_ROM = HEADER_SIZE + header.prgRomSize;
    chrRom.resize(header.chrRomSize);
    std::copy(data.begin() + START_CHR_ROM, data.begin() + START_CHR_ROM + header.chrRomSize, std::back_inserter(chrRom));

    // Read PlayChoice INST-ROM
    const size_t START_PLAYCHOICE_INST_ROM = HEADER_SIZE + header.prgRomSize + header.chrRomSize;
    playchoiceInstRom.resize(header.playchoiceInstRomSize);
    std::copy(data.begin() + START_PLAYCHOICE_INST_ROM, data.begin() + START_PLAYCHOICE_INST_ROM + header.playchoiceInstRomSize,
              std::back_inserter(playchoiceInstRom));

    // Read PlayChoice PROM
    const size_t START_PLAYCHOICE_PROM = HEADER_SIZE + header.prgRomSize + header.chrRomSize + header.playchoiceInstRomSize;
    playchoiceProm.resize(header.playchoicePromSize);
    std::copy(data.begin() + START_PLAYCHOICE_PROM, data.begin() + START_PLAYCHOICE_PROM + header.playchoicePromSize, std::back_inserter(playchoiceProm));

    // Read title
    const size_t START_TITLE = HEADER_SIZE + header.prgRomSize + header.chrRomSize + header.playchoiceInstRomSize + header.playchoicePromSize;
    title.reserve(TITLE_SIZE);
    std::copy(data.begin() + START_TITLE, data.end(), std::back_inserter(title));

    // Close file
    file.close();
}

}  // namespace iNES