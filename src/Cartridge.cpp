#include "Cartridge.hpp"
#include "Mapper0.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

struct INESHeader {
    char name[4];        // "NES\x1A"
    uint8_t prgRomChunks; // PRG ROM size in 16KB units
    uint8_t chrRomChunks; // CHR ROM size in 8KB units (0 = CHR RAM)
    uint8_t mapperLo;     // Flags 6: Mapper, screening, trainer, VS/Playchoice
    uint8_t mapperHi;     // Flags 7: Mapper, VS/Playchoice, NES 2.0 spec
    uint8_t prgRamSize;   // Flags 8: PRG RAM size (rarely used in iNES 1.0)
    uint8_t tvSystem1;    // Flags 9: TV system
    uint8_t tvSystem2;    // Flags 10: TV system, PRG RAM presence
    char unused[5];
};

Cartridge::Cartridge(const std::string& filename) {
    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return;
    }

    INESHeader header;
    ifs.read(reinterpret_cast<char*>(&header), sizeof(INESHeader));

    if (std::memcmp(header.name, "NES\x1A", 4) != 0) {
        std::cerr << "Invalid NES ROM file header: " << filename << std::endl;
        return;
    }

    if (header.mapperLo & 0x04) {
        // Skip trainer (512 bytes)
        ifs.seekg(512, std::ios_base::cur);
    }

    uint8_t mapperID = (header.mapperHi & 0xF0) | (header.mapperLo >> 4);

    // Check mirroring
    if (header.mapperLo & 0x08) {
        m_mirrorMode = MirrorMode::FOUR_SCREEN;
    } else if (header.mapperLo & 0x01) {
        m_mirrorMode = MirrorMode::VERTICAL;
    } else {
        m_mirrorMode = MirrorMode::HORIZONTAL;
    }

    // File type / iNES 1.0 or 2.0 detection
    m_prgBanks = header.prgRomChunks;
    m_chrBanks = header.chrRomChunks;

    m_prgMemory.resize(static_cast<size_t>(m_prgBanks) * 16384);
    ifs.read(reinterpret_cast<char*>(m_prgMemory.data()), m_prgMemory.size());

    if (m_chrBanks == 0) {
        // CHR RAM: allocate 8KB
        m_chrMemory.resize(8192, 0);
    } else {
        m_chrMemory.resize(static_cast<size_t>(m_chrBanks) * 8192);
        ifs.read(reinterpret_cast<char*>(m_chrMemory.data()), m_chrMemory.size());
    }

    // Instanciate mapper
    switch (mapperID) {
    case 0:
        m_mapper = std::make_shared<Mapper0>(m_prgBanks, m_chrBanks);
        break;
    default:
        std::cerr << "Unsupported mapper ID: " << static_cast<int>(mapperID) << std::endl;
        return;
    }

    m_loaded = true;
}

bool Cartridge::cpuRead(uint16_t addr, uint8_t &data) {
    uint32_t mapped_addr = 0;
    if (m_mapper && m_mapper->cpuMapRead(addr, mapped_addr)) {
        if (mapped_addr < m_prgMemory.size()) {
            data = m_prgMemory[mapped_addr];
            return true;
        }
    }
    return false;
}

bool Cartridge::cpuWrite(uint16_t addr, uint8_t data) {
    uint32_t mapped_addr = 0;
    if (m_mapper && m_mapper->cpuMapWrite(addr, mapped_addr, data)) {
        if (mapped_addr < m_prgMemory.size()) {
            m_prgMemory[mapped_addr] = data;
            return true;
        }
    }
    return false;
}

bool Cartridge::ppuRead(uint16_t addr, uint8_t &data) {
    uint32_t mapped_addr = 0;
    if (m_mapper && m_mapper->ppuMapRead(addr, mapped_addr)) {
        if (mapped_addr < m_chrMemory.size()) {
            data = m_chrMemory[mapped_addr];
            return true;
        }
    }
    return false;
}

bool Cartridge::ppuWrite(uint16_t addr, uint8_t data) {
    uint32_t mapped_addr = 0;
    if (m_mapper && m_mapper->ppuMapWrite(addr, mapped_addr)) {
        if (mapped_addr < m_chrMemory.size()) {
            m_chrMemory[mapped_addr] = data;
            return true;
        }
    }
    return false;
}

MirrorMode Cartridge::getMirrorMode() const {
    if (m_mapper) {
        MirrorMode mode = m_mapper->mirrorMode();
        if (mode != MirrorMode::HORIZONTAL) {
            return mode;
        }
    }
    return m_mirrorMode;
}
