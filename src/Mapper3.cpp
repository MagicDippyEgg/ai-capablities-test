#include "Mapper3.hpp"

Mapper3::Mapper3(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
    m_chrBank = 0;
}

bool Mapper3::cpuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        mapped_addr = addr & (m_prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper3::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) {
    (void)mapped_addr;
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        m_chrBank = data & 0x03;
    }
    return false;
}

bool Mapper3::ppuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        mapped_addr = (static_cast<uint32_t>(m_chrBank) * 0x2000) + addr;
        return true;
    }
    return false;
}

bool Mapper3::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        if (m_chrBanks == 0) {
            mapped_addr = addr;
            return true;
        }
    }
    return false;
}
