#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include "NES.hpp"

void createDummyNROMFile(const std::string& filename) {
    std::ofstream ofs(filename, std::ofstream::binary);

    uint8_t header[16] = {0};
    header[0] = 'N'; header[1] = 'E'; header[2] = 'S'; header[3] = 0x1A;
    header[4] = 2; // 32KB PRG
    header[5] = 1; // 8KB CHR
    header[6] = 0x00; // Mapper 0, horizontal mirroring
    header[7] = 0x00;

    ofs.write(reinterpret_cast<char*>(header), 16);

    std::vector<uint8_t> prg(32768, 0xEA); // NOP instructions
    // Infinite loop JMP $8000 at start
    prg[0] = 0x4C; prg[1] = 0x00; prg[2] = 0x80;

    // Reset vector -> $8000
    prg[32764] = 0x00; prg[32765] = 0x80;
    // NMI vector -> $8000
    prg[32762] = 0x00; prg[32763] = 0x80;

    ofs.write(reinterpret_cast<char*>(prg.data()), prg.size());

    std::vector<uint8_t> chr(8192, 0x00);
    ofs.write(reinterpret_cast<char*>(chr.data()), chr.size());
}

int main(int argc, char* argv[]) {
    std::string romPath = "test_nrom.nes";
    bool createdTemp = false;

    if (argc > 1) {
        romPath = argv[1];
    } else {
        createDummyNROMFile(romPath);
        createdTemp = true;
    }

    NES nes;
    if (!nes.loadCartridge(romPath)) {
        std::cerr << "Failed to load ROM: " << romPath << std::endl;
        if (createdTemp) std::remove(romPath.c_str());
        return 1;
    }

    std::cout << "Successfully loaded " << romPath << ". Running 120 frames..." << std::endl;

    for (int frame = 0; frame < 120; frame++) {
        while (!nes.getPPU().frameComplete) {
            nes.clock();
        }
        nes.getPPU().frameComplete = false;
    }

    if (createdTemp) {
        std::remove(romPath.c_str());
    }

    std::cout << "Successfully ran 120 frames of integration test!" << std::endl;
    return 0;
}
