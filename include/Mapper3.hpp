#ifndef MAPPER3_HPP
#define MAPPER3_HPP

#include "Mapper.hpp"

class Mapper3 : public Mapper {
public:
    Mapper3(uint8_t prgBanks, uint8_t chrBanks);
    ~Mapper3() override = default;

    bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data) override;
    bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override;

private:
    uint8_t m_chrBank = 0;
};

#endif // MAPPER3_HPP
