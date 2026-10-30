#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include "Cartridge.hpp"

void createDummyNESFile(const std::string& filename, uint8_t prgChunks, uint8_t chrChunks, uint8_t mapper) {
    std::ofstream ofs(filename, std::ofstream::binary);

    uint8_t header[16] = {0};
    header[0] = 'N';
    header[1] = 'E';
    header[2] = 'S';
    header[3] = 0x1A;
    header[4] = prgChunks;
    header[5] = chrChunks;
    header[6] = (mapper & 0x0F) << 4; // Mapper low + horizontal mirroring
    header[7] = mapper & 0xF0;        // Mapper high

    ofs.write(reinterpret_cast<char*>(header), 16);

    std::vector<uint8_t> prg(prgChunks * 16384, 0xEA); // NOPs
    if (prg.size() >= 2) {
        prg[prg.size() - 4] = 0x00; // Reset vector LSB
        prg[prg.size() - 3] = 0x80; // Reset vector MSB ($8000)
    }
    ofs.write(reinterpret_cast<char*>(prg.data()), prg.size());

    std::vector<uint8_t> chr(chrChunks * 8192, 0x55);
    ofs.write(reinterpret_cast<char*>(chr.data()), chr.size());
}

int main() {
    std::string testRomPath = "test_temp.nes";
    createDummyNESFile(testRomPath, 2, 1, 0); // Mapper 0, 32KB PRG, 8KB CHR

    Cartridge cart(testRomPath);
    assert(cart.isLoaded());

    // Test CPU reading
    uint8_t val = 0;
    bool readSuccess = cart.cpuRead(0x8000, val);
    assert(readSuccess);
    assert(val == 0xEA);

    // Test PPU reading
    uint8_t chrVal = 0;
    readSuccess = cart.ppuRead(0x0000, chrVal);
    assert(readSuccess);
    assert(chrVal == 0x55);

    // Clean up
    std::remove(testRomPath.c_str());

    std::cout << "Cartridge unit test passed!" << std::endl;
    return 0;
}
