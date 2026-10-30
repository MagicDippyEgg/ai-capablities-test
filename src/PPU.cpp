#include "PPU.hpp"
#include "Cartridge.hpp"
#include <cstring>
#include <iostream>

const std::array<uint32_t, 64> PPU::nesPalette = {
    0x7C7C7CFF, 0x0024BCFF, 0x0000D8FF, 0x4428BCFF, 0x940084FF, 0xA80020FF, 0xA81000FF, 0x881400FF,
    0x503000FF, 0x007800FF, 0x006800FF, 0x005800FF, 0x004058FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xBCBCBCFF, 0x0078F8FF, 0x0058F8FF, 0x6844FCFF, 0xD800CCFF, 0xE40058FF, 0xF83800FF, 0xE45C10FF,
    0xAC7C00FF, 0x00B800FF, 0x00A800FF, 0x00A844FF, 0x008888FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xF8F8F8FF, 0x3CBCFCFF, 0x6888FCFF, 0x9878F8FF, 0xF878F8FF, 0xF858B8FF, 0xF88858FF, 0xF8A000FF,
    0xF8B800FF, 0xB8F818FF, 0x58D854FF, 0x58F898FF, 0x00E8D8FF, 0x787878FF, 0x000000FF, 0x000000FF,
    0xFCFCFCFF, 0xA4E4FCFF, 0xB8B8FCFF, 0xD8B8FCFF, 0xF8B8FCFF, 0xF8A4C0FF, 0xF0D0B0FF, 0xFCE0A8FF,
    0xF8D878FF, 0xD8F878FF, 0xB8F8B8FF, 0xB8F8D8FF, 0x00FCFCFF, 0xF8D8F8FF, 0x000000FF, 0x000000FF
};

PPU::PPU() {
    frameBuffer.fill(0x000000FF);
    m_nameTable[0].fill(0x00);
    m_nameTable[1].fill(0x00);
    m_paletteTable.fill(0x00);
    m_oam.fill(0x00);
}

PPU::~PPU() = default;

void PPU::connectCartridge(std::shared_ptr<Cartridge> cartridge) {
    m_cartridge = cartridge;
}

uint8_t PPU::cpuRead(uint16_t addr, bool readOnly) {
    uint8_t data = 0x00;

    if (readOnly) {
        switch (addr) {
        case 0x0000: data = m_control.reg; break;
        case 0x0001: data = m_mask.reg; break;
        case 0x0002: data = m_status.reg; break;
        case 0x0003: break;
        case 0x0004: data = m_oam[m_oamAddr]; break;
        case 0x0005: break;
        case 0x0006: break;
        case 0x0007: data = ppuRead(m_vramAddr.reg); break;
        }
    } else {
        switch (addr) {
        case 0x0000: // Control - Write only
            break;
        case 0x0001: // Mask - Write only
            break;
        case 0x0002: // Status
            data = (m_status.reg & 0xE0) | (m_ppuDataBuffer & 0x1F);
            m_status.vertical_blank = 0;
            m_addressLatch = 0;
            break;
        case 0x0003: // OAM Address - Write only
            break;
        case 0x0004: // OAM Data
            data = m_oam[m_oamAddr];
            break;
        case 0x0005: // Scroll - Write only
            break;
        case 0x0006: // PPU Address - Write only
            break;
        case 0x0007: // PPU Data
            data = m_ppuDataBuffer;
            m_ppuDataBuffer = ppuRead(m_vramAddr.reg);

            if (m_vramAddr.reg >= 0x3F00) {
                data = m_ppuDataBuffer;
            }
            m_vramAddr.reg += (m_control.increment_mode ? 32 : 1);
            break;
        }
    }

    return data;
}

void PPU::cpuWrite(uint16_t addr, uint8_t data) {
    switch (addr) {
    case 0x0000: // Control
        m_control.reg = data;
        m_tramAddr.nametable_x = m_control.nametable_x;
        m_tramAddr.nametable_y = m_control.nametable_y;
        break;
    case 0x0001: // Mask
        m_mask.reg = data;
        break;
    case 0x0002: // Status
        break;
    case 0x0003: // OAM Address
        m_oamAddr = data;
        break;
    case 0x0004: // OAM Data
        m_oam[m_oamAddr] = data;
        m_oamAddr++;
        break;
    case 0x0005: // Scroll
        if (m_addressLatch == 0) {
            m_fineX = data & 0x07;
            m_tramAddr.coarse_x = data >> 3;
            m_addressLatch = 1;
        } else {
            m_tramAddr.fine_y = data & 0x07;
            m_tramAddr.coarse_y = data >> 3;
            m_addressLatch = 0;
        }
        break;
    case 0x0006: // PPU Address
        if (m_addressLatch == 0) {
            m_tramAddr.reg = (uint16_t((data & 0x3F)) << 8) | (m_tramAddr.reg & 0x00FF);
            m_addressLatch = 1;
        } else {
            m_tramAddr.reg = (m_tramAddr.reg & 0xFF00) | data;
            m_vramAddr = m_tramAddr;
            m_addressLatch = 0;
        }
        break;
    case 0x0007: // PPU Data
        ppuWrite(m_vramAddr.reg, data);
        m_vramAddr.reg += (m_control.increment_mode ? 32 : 1);
        break;
    }
}

uint8_t PPU::ppuRead(uint16_t addr) {
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if (m_cartridge && m_cartridge->ppuRead(addr, data)) {
        // Read from cartridge CHR ROM/RAM
    } else if (addr <= 0x1FFF) {
        // CHR RAM/ROM space
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        MirrorMode mirror = m_cartridge ? m_cartridge->getMirrorMode() : MirrorMode::HORIZONTAL;

        if (mirror == MirrorMode::VERTICAL) {
            if (addr <= 0x03FF) data = m_nameTable[0][addr & 0x03FF];
            else if (addr <= 0x07FF) data = m_nameTable[1][addr & 0x03FF];
            else if (addr <= 0x0BFF) data = m_nameTable[0][addr & 0x03FF];
            else data = m_nameTable[1][addr & 0x03FF];
        } else if (mirror == MirrorMode::HORIZONTAL) {
            if (addr <= 0x03FF) data = m_nameTable[0][addr & 0x03FF];
            else if (addr <= 0x07FF) data = m_nameTable[0][addr & 0x03FF];
            else if (addr <= 0x0BFF) data = m_nameTable[1][addr & 0x03FF];
            else data = m_nameTable[1][addr & 0x03FF];
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        data = m_paletteTable[addr] & (m_mask.grayscale ? 0x30 : 0x3F);
    }

    return data;
}

void PPU::ppuWrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF;

    if (m_cartridge && m_cartridge->ppuWrite(addr, data)) {
        // Written to cartridge CHR
    } else if (addr <= 0x1FFF) {
        // CHR pattern table
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        MirrorMode mirror = m_cartridge ? m_cartridge->getMirrorMode() : MirrorMode::HORIZONTAL;

        if (mirror == MirrorMode::VERTICAL) {
            if (addr <= 0x03FF) m_nameTable[0][addr & 0x03FF] = data;
            else if (addr <= 0x07FF) m_nameTable[1][addr & 0x03FF] = data;
            else if (addr <= 0x0BFF) m_nameTable[0][addr & 0x03FF] = data;
            else m_nameTable[1][addr & 0x03FF] = data;
        } else if (mirror == MirrorMode::HORIZONTAL) {
            if (addr <= 0x03FF) m_nameTable[0][addr & 0x03FF] = data;
            else if (addr <= 0x07FF) m_nameTable[0][addr & 0x03FF] = data;
            else if (addr <= 0x0BFF) m_nameTable[1][addr & 0x03FF] = data;
            else m_nameTable[1][addr & 0x03FF] = data;
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        m_paletteTable[addr] = data;
    }
}

void PPU::incrementScrollX() {
    if (m_mask.render_background || m_mask.render_sprites) {
        if (m_vramAddr.coarse_x == 31) {
            m_vramAddr.coarse_x = 0;
            m_vramAddr.nametable_x = ~m_vramAddr.nametable_x;
        } else {
            m_vramAddr.coarse_x++;
        }
    }
}

void PPU::incrementScrollY() {
    if (m_mask.render_background || m_mask.render_sprites) {
        if (m_vramAddr.fine_y < 7) {
            m_vramAddr.fine_y++;
        } else {
            m_vramAddr.fine_y = 0;
            if (m_vramAddr.coarse_y == 29) {
                m_vramAddr.coarse_y = 0;
                m_vramAddr.nametable_y = ~m_vramAddr.nametable_y;
            } else if (m_vramAddr.coarse_y == 31) {
                m_vramAddr.coarse_y = 0;
            } else {
                m_vramAddr.coarse_y++;
            }
        }
    }
}

void PPU::transferAddressX() {
    if (m_mask.render_background || m_mask.render_sprites) {
        m_vramAddr.nametable_x = m_tramAddr.nametable_x;
        m_vramAddr.coarse_x = m_tramAddr.coarse_x;
    }
}

void PPU::transferAddressY() {
    if (m_mask.render_background || m_mask.render_sprites) {
        m_vramAddr.fine_y = m_tramAddr.fine_y;
        m_vramAddr.nametable_y = m_tramAddr.nametable_y;
        m_vramAddr.coarse_y = m_tramAddr.coarse_y;
    }
}

void PPU::loadBackgroundShifters() {
    m_bgShifterPatternLo = (m_bgShifterPatternLo & 0xFF00) | m_bgNextTileLsb;
    m_bgShifterPatternHi = (m_bgShifterPatternHi & 0xFF00) | m_bgNextTileMsb;

    m_bgShifterAttribLo = (m_bgShifterAttribLo & 0xFF00) | ((m_bgNextTileAttrib & 0b01) ? 0xFF : 0x00);
    m_bgShifterAttribHi = (m_bgShifterAttribHi & 0xFF00) | ((m_bgNextTileAttrib & 0b10) ? 0xFF : 0x00);
}

void PPU::updateShifters() {
    if (m_mask.render_background) {
        m_bgShifterPatternLo <<= 1;
        m_bgShifterPatternHi <<= 1;
        m_bgShifterAttribLo <<= 1;
        m_bgShifterAttribHi <<= 1;
    }

    if (m_mask.render_sprites && m_cycle >= 1 && m_cycle < 257) {
        for (int i = 0; i < m_spriteCount; i++) {
            if (m_spriteScanline[i].x > 0) {
                m_spriteScanline[i].x--;
            } else {
                m_spriteShifterPatternLo[i] <<= 1;
                m_spriteShifterPatternHi[i] <<= 1;
            }
        }
    }
}

void PPU::reset() {
    m_fineX = 0x00;
    m_addressLatch = 0x00;
    m_ppuDataBuffer = 0x00;
    m_scanline = 0;
    m_cycle = 0;
    m_bgNextTileId = 0x00;
    m_bgNextTileAttrib = 0x00;
    m_bgNextTileLsb = 0x00;
    m_bgNextTileMsb = 0x00;
    m_bgShifterPatternLo = 0x0000;
    m_bgShifterPatternHi = 0x0000;
    m_bgShifterAttribLo = 0x0000;
    m_bgShifterAttribHi = 0x0000;
    m_status.reg = 0x00;
    m_mask.reg = 0x00;
    m_control.reg = 0x00;
    m_vramAddr.reg = 0x0000;
    m_tramAddr.reg = 0x0000;
}

void PPU::clock() {
    if (m_scanline >= -1 && m_scanline < 240) {
        if (m_scanline == -1 && m_cycle == 1) {
            m_status.vertical_blank = 0;
            m_status.sprite_overflow = 0;
            m_status.sprite_zero_hit = 0;
            for (int i = 0; i < 8; i++) {
                m_spriteShifterPatternLo[i] = 0;
                m_spriteShifterPatternHi[i] = 0;
            }
        }

        if ((m_cycle >= 2 && m_cycle < 258) || (m_cycle >= 321 && m_cycle < 338)) {
            updateShifters();

            switch ((m_cycle - 1) % 8) {
            case 0:
                loadBackgroundShifters();
                m_bgNextTileId = ppuRead(0x2000 | (m_vramAddr.reg & 0x0FFF));
                break;
            case 2:
                m_bgNextTileAttrib = ppuRead(0x23C0 | (m_vramAddr.nametable_y << 11)
                                                     | (m_vramAddr.nametable_x << 10)
                                                     | ((m_vramAddr.coarse_y >> 2) << 3)
                                                     | (m_vramAddr.coarse_x >> 2));
                if (m_vramAddr.coarse_y & 0x02) m_bgNextTileAttrib >>= 4;
                if (m_vramAddr.coarse_x & 0x02) m_bgNextTileAttrib >>= 2;
                m_bgNextTileAttrib &= 0x03;
                break;
            case 4:
                m_bgNextTileLsb = ppuRead((m_control.pattern_background << 12)
                                         + ((uint16_t)m_bgNextTileId << 4)
                                         + (m_vramAddr.fine_y + 0));
                break;
            case 6:
                m_bgNextTileMsb = ppuRead((m_control.pattern_background << 12)
                                         + ((uint16_t)m_bgNextTileId << 4)
                                         + (m_vramAddr.fine_y + 8));
                break;
            case 7:
                incrementScrollX();
                break;
            }
        }

        if (m_cycle == 256) {
            incrementScrollY();
        }

        if (m_cycle == 257) {
            loadBackgroundShifters();
            transferAddressX();
        }

        if (m_scanline == -1 && m_cycle >= 280 && m_cycle < 305) {
            transferAddressY();
        }

        // Sprite Evaluation at scanline end (cycle 257)
        if (m_cycle == 257 && m_scanline >= 0) {
            std::memset(m_spriteScanline.data(), 0xFF, m_spriteScanline.size() * sizeof(ObjectAttributeEntry));
            m_spriteCount = 0;
            for (uint8_t i = 0; i < 8; i++) {
                m_spriteShifterPatternLo[i] = 0;
                m_spriteShifterPatternHi[i] = 0;
            }

            uint8_t oamEntry = 0;
            m_spriteZeroHitPossible = false;

            while (oamEntry < 64 && m_spriteCount < 9) {
                int16_t diff = ((int16_t)m_scanline - (int16_t)m_oam[oamEntry * 4]);
                int16_t spriteHeight = m_control.sprite_size ? 16 : 8;

                if (diff >= 0 && diff < spriteHeight) {
                    if (m_spriteCount < 8) {
                        if (oamEntry == 0) {
                            m_spriteZeroHitPossible = true;
                        }

                        m_spriteScanline[m_spriteCount].y = m_oam[oamEntry * 4 + 0];
                        m_spriteScanline[m_spriteCount].id = m_oam[oamEntry * 4 + 1];
                        m_spriteScanline[m_spriteCount].attribute = m_oam[oamEntry * 4 + 2];
                        m_spriteScanline[m_spriteCount].x = m_oam[oamEntry * 4 + 3];
                        m_spriteCount++;
                    }
                }
                oamEntry++;
            }
            m_status.sprite_overflow = (m_spriteCount > 8) ? 1 : 0;
        }

        if (m_cycle == 340) {
            for (uint8_t i = 0; i < m_spriteCount; i++) {
                uint8_t spritePatternBitsLo, spritePatternBitsHi;
                uint16_t spritePatternAddrLo, spritePatternAddrHi;

                if (!m_control.sprite_size) { // 8x8 Mode
                    if (!(m_spriteScanline[i].attribute & 0x80)) { // Normal vertical
                        spritePatternAddrLo = (m_control.pattern_sprite << 12)
                                            | (m_spriteScanline[i].id << 4)
                                            | (m_scanline - m_spriteScanline[i].y);
                    } else { // Flipped vertical
                        spritePatternAddrLo = (m_control.pattern_sprite << 12)
                                            | (m_spriteScanline[i].id << 4)
                                            | (7 - (m_scanline - m_spriteScanline[i].y));
                    }
                } else { // 8x16 Mode
                    if (!(m_spriteScanline[i].attribute & 0x80)) {
                        if (m_scanline - m_spriteScanline[i].y < 8) { // Top half
                            spritePatternAddrLo = ((m_spriteScanline[i].id & 0x01) << 12)
                                                | ((m_spriteScanline[i].id & 0xFE) << 4)
                                                | ((m_scanline - m_spriteScanline[i].y) & 0x07);
                        } else { // Bottom half
                            spritePatternAddrLo = ((m_spriteScanline[i].id & 0x01) << 12)
                                                | (((m_spriteScanline[i].id & 0xFE) + 1) << 4)
                                                | ((m_scanline - m_spriteScanline[i].y) & 0x07);
                        }
                    } else {
                        if (m_scanline - m_spriteScanline[i].y < 8) { // Top half
                            spritePatternAddrLo = ((m_spriteScanline[i].id & 0x01) << 12)
                                                | (((m_spriteScanline[i].id & 0xFE) + 1) << 4)
                                                | ((7 - (m_scanline - m_spriteScanline[i].y)) & 0x07);
                        } else { // Bottom half
                            spritePatternAddrLo = ((m_spriteScanline[i].id & 0x01) << 12)
                                                | ((m_spriteScanline[i].id & 0xFE) << 4)
                                                | ((7 - (m_scanline - m_spriteScanline[i].y)) & 0x07);
                        }
                    }
                }

                spritePatternAddrHi = spritePatternAddrLo + 8;
                spritePatternBitsLo = ppuRead(spritePatternAddrLo);
                spritePatternBitsHi = ppuRead(spritePatternAddrHi);

                if (m_spriteScanline[i].attribute & 0x40) { // Horizontal flip
                    auto flipByte = [](uint8_t b) {
                        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
                        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
                        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
                        return b;
                    };
                    spritePatternBitsLo = flipByte(spritePatternBitsLo);
                    spritePatternBitsHi = flipByte(spritePatternBitsHi);
                }

                m_spriteShifterPatternLo[i] = spritePatternBitsLo;
                m_spriteShifterPatternHi[i] = spritePatternBitsHi;
            }
        }
    }

    if (m_scanline == 240) {
        // Post render scanline - idle
    }

    if (m_scanline >= 241 && m_scanline < 261) {
        if (m_scanline == 241 && m_cycle == 1) {
            m_status.vertical_blank = 1;
            if (m_control.generate_nmi) {
                nmi = true;
            }
        }
    }

    // Pixel Rendering
    uint8_t bgPixel = 0x00;
    uint8_t bgPalette = 0x00;

    if (m_mask.render_background) {
        uint16_t bitMux = 0x8000 >> m_fineX;

        uint8_t p0Pixel = (m_bgShifterPatternLo & bitMux) ? 1 : 0;
        uint8_t p1Pixel = (m_bgShifterPatternHi & bitMux) ? 1 : 0;
        bgPixel = (p1Pixel << 1) | p0Pixel;

        uint8_t bgPal0 = (m_bgShifterAttribLo & bitMux) ? 1 : 0;
        uint8_t bgPal1 = (m_bgShifterAttribHi & bitMux) ? 1 : 0;
        bgPalette = (bgPal1 << 1) | bgPal0;
    }

    uint8_t fgPixel = 0x00;
    uint8_t fgPalette = 0x00;
    uint8_t fgPriority = 0x00;

    if (m_mask.render_sprites) {
        m_spriteZeroBeingRendered = false;

        for (uint8_t i = 0; i < m_spriteCount; i++) {
            if (m_spriteScanline[i].x == 0) {
                uint8_t fgPixelLo = (m_spriteShifterPatternLo[i] & 0x80) ? 1 : 0;
                uint8_t fgPixelHi = (m_spriteShifterPatternHi[i] & 0x80) ? 1 : 0;
                fgPixel = (fgPixelHi << 1) | fgPixelLo;

                fgPalette = (m_spriteScanline[i].attribute & 0x03) + 0x04;
                fgPriority = (m_spriteScanline[i].attribute & 0x20) == 0;

                if (fgPixel != 0) {
                    if (i == 0) {
                        m_spriteZeroBeingRendered = true;
                    }
                    break;
                }
            }
        }
    }

    uint8_t pixel = 0x00;
    uint8_t palette = 0x00;

    if (bgPixel == 0 && fgPixel == 0) {
        pixel = 0x00;
        palette = 0x00;
    } else if (bgPixel == 0 && fgPixel > 0) {
        pixel = fgPixel;
        palette = fgPalette;
    } else if (bgPixel > 0 && fgPixel == 0) {
        pixel = bgPixel;
        palette = bgPalette;
    } else if (bgPixel > 0 && fgPixel > 0) {
        if (fgPriority) {
            pixel = fgPixel;
            palette = fgPalette;
        } else {
            pixel = bgPixel;
            palette = bgPalette;
        }

        if (m_spriteZeroHitPossible && m_spriteZeroBeingRendered) {
            if (m_mask.render_background && m_mask.render_sprites) {
                if (~(m_mask.render_background_left | m_mask.render_sprites_left)) {
                    if (m_cycle >= 9 && m_cycle < 258) {
                        m_status.sprite_zero_hit = 1;
                    }
                } else {
                    if (m_cycle >= 1 && m_cycle < 258) {
                        m_status.sprite_zero_hit = 1;
                    }
                }
            }
        }
    }

    if (m_scanline >= 0 && m_scanline < 240 && m_cycle >= 1 && m_cycle <= 256) {
        uint8_t colorIndex = ppuRead(0x3F00 + (palette << 2) + pixel);
        frameBuffer[m_scanline * 256 + (m_cycle - 1)] = nesPalette[colorIndex & 0x3F];
    }

    // Advance cycle and scanline
    m_cycle++;
    if (m_cycle >= 341) {
        m_cycle = 0;
        m_scanline++;
        if (m_scanline >= 261) {
            m_scanline = -1;
            frameComplete = true;
        }
    }
}
