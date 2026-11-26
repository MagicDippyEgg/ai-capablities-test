#ifndef MAPPER2_HPP
#define MAPPER2_HPP

#include "Mapper.hpp"

class Mapper2 : public Mapper {
public:
    Mapper2(uint8_t prgBanks, uint8_t chrBanks);
    ~Mapper2() override = default;

    bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) override;
    bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;

private:
    uint8_t m_prgBankLo = 0;
};

#endif // MAPPER2_HPP
