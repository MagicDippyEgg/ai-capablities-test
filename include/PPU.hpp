#ifndef PPU_HPP
#define PPU_HPP

#include <cstdint>
#include <memory>
#include <array>
#include "Mapper.hpp"

class Cartridge;

class PPU {
public:
    PPU();
    ~PPU();

    void connectCartridge(std::shared_ptr<Cartridge> cartridge);

    uint8_t cpuRead(uint16_t addr, bool readOnly = false);
    void cpuWrite(uint16_t addr, uint8_t data);

    uint8_t ppuRead(uint16_t addr);
    void ppuWrite(uint16_t addr, uint8_t data);

    void clock();
    void reset();

    bool nmi = false;
    bool frameComplete = false;

    // Output frame buffer (256 x 240, 32-bit RGBA)
    std::array<uint32_t, 256 * 240> frameBuffer{};

    uint8_t* getOAMPtr() { return m_oam.data(); }

private:
    std::shared_ptr<Cartridge> m_cartridge;

    std::array<std::array<uint8_t, 1024>, 2> m_nameTable{};
    std::array<uint8_t, 32> m_paletteTable{};

    int16_t m_scanline = 0;
    int16_t m_cycle = 0;

    // PPU Registers
    union PPUCTRL {
        struct {
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t increment_mode : 1;
            uint8_t pattern_sprite : 1;
            uint8_t pattern_background : 1;
            uint8_t sprite_size : 1;
            uint8_t master_slave : 1;
            uint8_t generate_nmi : 1;
        };
        uint8_t reg;
    } m_control{};

    union PPUMASK {
        struct {
            uint8_t grayscale : 1;
            uint8_t render_background_left : 1;
            uint8_t render_sprites_left : 1;
            uint8_t render_background : 1;
            uint8_t render_sprites : 1;
            uint8_t enhance_red : 1;
            uint8_t enhance_green : 1;
            uint8_t enhance_blue : 1;
        };
        uint8_t reg;
    } m_mask{};

    union PPUSTATUS {
        struct {
            uint8_t unused : 5;
            uint8_t sprite_overflow : 1;
            uint8_t sprite_zero_hit : 1;
            uint8_t vertical_blank : 1;
        };
        uint8_t reg;
    } m_status{};

    // Loopy registers
    union LoopyRegister {
        struct {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y : 3;
            uint16_t unused : 1;
        };
        uint16_t reg = 0x0000;
    };

    LoopyRegister m_vramAddr;
    LoopyRegister m_tramAddr;
    uint8_t m_fineX = 0x00;
    uint8_t m_addressLatch = 0x00;
    uint8_t m_ppuDataBuffer = 0x00;

    // Background rendering helpers
    uint8_t m_bgNextTileId = 0x00;
    uint8_t m_bgNextTileAttrib = 0x00;
    uint8_t m_bgNextTileLsb = 0x00;
    uint8_t m_bgNextTileMsb = 0x00;

    uint16_t m_bgShifterPatternLo = 0x0000;
    uint16_t m_bgShifterPatternHi = 0x0000;
    uint16_t m_bgShifterAttribLo = 0x0000;
    uint16_t m_bgShifterAttribHi = 0x0000;

    void incrementScrollX();
    void incrementScrollY();
    void transferAddressX();
    void transferAddressY();
    void loadBackgroundShifters();
    void updateShifters();

    // OAM & Sprites
    struct ObjectAttributeEntry {
        uint8_t y;
        uint8_t id;
        uint8_t attribute;
        uint8_t x;
    };

    std::array<uint8_t, 256> m_oam{};
    uint8_t m_oamAddr = 0x00;

    std::array<ObjectAttributeEntry, 8> m_spriteScanline{};
    uint8_t m_spriteCount = 0;
    std::array<uint8_t, 8> m_spriteShifterPatternLo{};
    std::array<uint8_t, 8> m_spriteShifterPatternHi{};
    bool m_spriteZeroHitPossible = false;
    bool m_spriteZeroBeingRendered = false;

    // Colors palette (RGB)
    static const std::array<uint32_t, 64> nesPalette;
};

#endif // PPU_HPP
