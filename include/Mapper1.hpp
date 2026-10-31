#ifndef MAPPER1_HPP
#define MAPPER1_HPP

#include "Mapper.hpp"

class Mapper1 : public Mapper {
public:
    Mapper1(uint8_t prgBanks, uint8_t chrBanks);
    ~Mapper1() override = default;

    bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) override;
    bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;

    MirrorMode mirrorMode() const override;

private:
    uint8_t m_shiftRegister = 0x00;
    uint8_t m_writeCount = 0;
    uint8_t m_controlRegister = 0x1C; // Mode 3 by default
    uint8_t m_chrBank0 = 0x00;
    uint8_t m_chrBank1 = 0x00;
    uint8_t m_prgBank = 0x00;
};

#endif // MAPPER1_HPP
