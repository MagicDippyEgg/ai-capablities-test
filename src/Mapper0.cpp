#include "Mapper0.hpp"

Mapper0::Mapper0(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}

bool Mapper0::cpuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        mapped_addr = addr & (m_prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper0::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) {
    (void)data;
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        mapped_addr = addr & (m_prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper0::ppuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        mapped_addr = addr;
        return true;
    }
    return false;
}

bool Mapper0::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        if (m_chrBanks == 0) {
            mapped_addr = addr;
            return true;
        }
    }
    return false;
}
