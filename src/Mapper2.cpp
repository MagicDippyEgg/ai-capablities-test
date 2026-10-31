#include "Mapper2.hpp"

Mapper2::Mapper2(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
    m_prgBankLo = 0;
}

bool Mapper2::cpuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr >= 0x8000 && addr <= 0xBFFF) {
        mapped_addr = (static_cast<uint32_t>(m_prgBankLo) * 0x4000) + (addr & 0x3FFF);
        return true;
    } else if (addr >= 0xC000 && addr <= 0xFFFF) {
        mapped_addr = (static_cast<uint32_t>(m_prgBanks - 1) * 0x4000) + (addr & 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper2::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) {
    (void)mapped_addr;
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        m_prgBankLo = data & 0x0F;
    }
    return false;
}

bool Mapper2::ppuMapRead(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        mapped_addr = addr;
        return true;
    }
    return false;
}

bool Mapper2::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) {
    if (addr <= 0x1FFF) {
        if (m_chrBanks == 0) {
            mapped_addr = addr;
            return true;
        }
    }
    return false;
}
