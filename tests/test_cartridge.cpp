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
    header[6] = (mapper & 0x0F) << 4; // Mapper low
    header[7] = mapper & 0xF0;        // Mapper high

    ofs.write(reinterpret_cast<char*>(header), 16);

    std::vector<uint8_t> prg(prgChunks * 16384, 0xEA);
    for (size_t i = 0; i < 16384; i++) {
        prg[i] = 0x11; // Bank 0
    }
    if (prgChunks == 2) {
        for (size_t i = 16384; i < 32768; i++) {
            prg[i] = 0x22; // Bank 1
        }
    } else if (prgChunks > 2) {
        for (size_t i = 16384; i < 32768; i++) {
            prg[i] = 0x22; // Bank 1
        }
        for (size_t i = prg.size() - 16384; i < prg.size(); i++) {
            prg[i] = 0x99; // Last bank
        }
    }
    ofs.write(reinterpret_cast<char*>(prg.data()), prg.size());

    std::vector<uint8_t> chr(chrChunks * 8192, 0x55);
    if (chr.size() >= 16384) {
        for (size_t i = 8192; i < 16384; i++) {
            chr[i] = 0x77; // CHR Bank 1
        }
    }
    ofs.write(reinterpret_cast<char*>(chr.data()), chr.size());
}

void testMapper0() {
    std::string testRomPath = "test_mapper0.nes";
    createDummyNESFile(testRomPath, 2, 1, 0); // Mapper 0, 32KB PRG, 8KB CHR

    Cartridge cart(testRomPath);
    assert(cart.isLoaded());

    uint8_t val = 0;
    bool readSuccess = cart.cpuRead(0x8000, val);
    assert(readSuccess);
    assert(val == 0x11);

    readSuccess = cart.cpuRead(0xC000, val);
    assert(readSuccess);
    assert(val == 0x22);

    std::remove(testRomPath.c_str());
    std::cout << "testMapper0 passed!" << std::endl;
}

void testMapper1_And_PRGRAM() {
    std::string testRomPath = "test_mapper1.nes";
    createDummyNESFile(testRomPath, 4, 2, 1); // Mapper 1, 64KB PRG, 16KB CHR

    Cartridge cart(testRomPath);
    assert(cart.isLoaded());

    // Test PRG RAM ($6000-$7FFF) read and write
    cart.cpuWrite(0x6000, 0x42);
    uint8_t ramVal = 0;
    bool ramRead = cart.cpuRead(0x6000, ramVal);
    assert(ramRead);
    assert(ramVal == 0x42);

    uint8_t val = 0;
    cart.cpuRead(0x8000, val);
    assert(val == 0x11);
    cart.cpuRead(0xC000, val);
    assert(val == 0x99);

    std::remove(testRomPath.c_str());
    std::cout << "testMapper1_And_PRGRAM passed!" << std::endl;
}

void testMapper2_UxROM() {
    std::string testRomPath = "test_mapper2.nes";
    createDummyNESFile(testRomPath, 4, 0, 2); // Mapper 2, 64KB PRG, 0 CHR (RAM)

    Cartridge cart(testRomPath);
    assert(cart.isLoaded());

    uint8_t val = 0;
    cart.cpuRead(0x8000, val);
    assert(val == 0x11); // Initial bank 0 at $8000
    cart.cpuRead(0xC000, val);
    assert(val == 0x99); // Fixed last bank at $C000

    // Switch bank 1 into $8000
    cart.cpuWrite(0x8000, 0x01);
    cart.cpuRead(0x8000, val);
    assert(val == 0x22); // Switched to bank 1
    cart.cpuRead(0xC000, val);
    assert(val == 0x99); // Fixed last bank unchanged

    std::remove(testRomPath.c_str());
    std::cout << "testMapper2_UxROM passed!" << std::endl;
}

void testMapper3_CNROM() {
    std::string testRomPath = "test_mapper3.nes";
    createDummyNESFile(testRomPath, 2, 2, 3); // Mapper 3, 32KB PRG, 16KB CHR

    Cartridge cart(testRomPath);
    assert(cart.isLoaded());

    uint8_t chrVal = 0;
    cart.ppuRead(0x0000, chrVal);
    assert(chrVal == 0x55); // CHR bank 0

    // Switch CHR bank to 1
    cart.cpuWrite(0x8000, 0x01);
    cart.ppuRead(0x0000, chrVal);
    assert(chrVal == 0x77); // CHR bank 1

    std::remove(testRomPath.c_str());
    std::cout << "testMapper3_CNROM passed!" << std::endl;
}

void testMapper4_MMC3() {
    std::string testRomPath = "test_mapper4.nes";
    createDummyNESFile(testRomPath, 8, 4, 4); // Mapper 4, 128KB PRG, 32KB CHR

    Cartridge cart(testRomPath);
    assert(cart.isLoaded());

    // Test MMC3 Scanline IRQ
    // Set IRQ latch to 3
    cart.cpuWrite(0xC000, 3);
    // Reload IRQ
    cart.cpuWrite(0xC001, 0);
    // Enable IRQ
    cart.cpuWrite(0xE001, 0);

    assert(!cart.irqActive());
    cart.scanline(); // reload -> 3
    assert(!cart.irqActive());
    cart.scanline(); // counter: 3 -> 2
    assert(!cart.irqActive());
    cart.scanline(); // counter: 2 -> 1
    assert(!cart.irqActive());
    cart.scanline(); // counter: 1 -> 0, IRQ triggers!
    assert(cart.irqActive());

    cart.irqClear();
    assert(!cart.irqActive());

    std::remove(testRomPath.c_str());
    std::cout << "testMapper4_MMC3 passed!" << std::endl;
}

int main() {
    testMapper0();
    testMapper1_And_PRGRAM();
    testMapper2_UxROM();
    testMapper3_CNROM();
    testMapper4_MMC3();
    std::cout << "All cartridge unit tests passed!" << std::endl;
    return 0;
}
