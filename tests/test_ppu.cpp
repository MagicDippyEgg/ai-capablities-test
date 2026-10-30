#include <iostream>
#include <cassert>
#include "PPU.hpp"

void testPPU_Registers() {
    PPU ppu;

    // Test Control register write / Loopy t setup
    ppu.cpuWrite(0x0000, 0x90); // NMI enable (0x80) + Pattern Table 1 for BG (0x10)
    // Read status reg clears latch
    ppu.cpuRead(0x0002, false);

    // Write address $2105
    ppu.cpuWrite(0x0006, 0x21);
    ppu.cpuWrite(0x0006, 0x05);

    // Write data to $2105
    ppu.cpuWrite(0x0007, 0xAA);

    // Reset address latch and set address back to $2105
    ppu.cpuRead(0x0002, false);
    ppu.cpuWrite(0x0006, 0x21);
    ppu.cpuWrite(0x0006, 0x05);

    // Read dummy buffer
    ppu.cpuRead(0x0007, false);
    // Read actual value
    uint8_t val = ppu.cpuRead(0x0007, false);
    assert(val == 0xAA);

    std::cout << "testPPU_Registers passed!" << std::endl;
}

void testPPU_VBlankAndNMI() {
    PPU ppu;
    ppu.cpuWrite(0x0000, 0x80); // Enable NMI on VBlank

    // Clock until scanline 241, cycle 1
    for (int line = -1; line < 241; line++) {
        for (int cycle = 0; cycle < 341; cycle++) {
            ppu.clock();
        }
    }
    // Step cycle 1 on scanline 241
    ppu.clock();

    assert(ppu.nmi == true);
    uint8_t status = ppu.cpuRead(0x0002, false);
    assert((status & 0x80) != 0); // VBlank flag set

    std::cout << "testPPU_VBlankAndNMI passed!" << std::endl;
}

int main() {
    testPPU_Registers();
    testPPU_VBlankAndNMI();
    std::cout << "All PPU unit tests passed!" << std::endl;
    return 0;
}
