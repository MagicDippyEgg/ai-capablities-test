#include "Bus.hpp"
#include "CPU.hpp"
#include "PPU.hpp"
#include "APU.hpp"
#include "Cartridge.hpp"
#include "Controller.hpp"

Bus::Bus() {
    ram.fill(0x00);
}

Bus::~Bus() = default;

void Bus::connectCPU(CPU* cpu) {
    m_cpu = cpu;
}

void Bus::connectPPU(PPU* ppu) {
    m_ppu = ppu;
}

void Bus::connectAPU(APU* apu) {
    m_apu = apu;
}

void Bus::insertCartridge(std::shared_ptr<Cartridge> cartridge) {
    m_cartridge = cartridge;
    if (m_ppu) {
        m_ppu->connectCartridge(cartridge);
    }
}

void Bus::connectControllers(Controller* pad1, Controller* pad2) {
    m_controller1 = pad1;
    m_controller2 = pad2;
}

uint8_t Bus::cpuRead(uint16_t addr, bool readOnly) {
    uint8_t data = 0x00;

    if (m_cartridge && m_cartridge->cpuRead(addr, data)) {
        // Handled by cartridge
    } else if (addr <= 0x1FFF) {
        // System RAM (2KB mirrored every 2KB up to $1FFF)
        data = ram[addr & 0x07FF];
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // PPU Registers (mirrored every 8 bytes)
        if (m_ppu) {
            data = m_ppu->cpuRead(addr & 0x0007, readOnly);
        }
    } else if (addr == 0x4016) {
        if (m_controller1) {
            data = m_controller1->read();
        }
    } else if (addr == 0x4017) {
        if (m_controller2) {
            data = m_controller2->read();
        }
    } else if (addr >= 0x4000 && addr <= 0x4015) {
        if (m_apu) {
            data = m_apu->cpuRead(addr);
        }
    }

    return data;
}

void Bus::cpuWrite(uint16_t addr, uint8_t data) {
    if (m_cartridge && m_cartridge->cpuWrite(addr, data)) {
        // Handled by cartridge
    } else if (addr <= 0x1FFF) {
        ram[addr & 0x07FF] = data;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        if (m_ppu) {
            m_ppu->cpuWrite(addr & 0x0007, data);
        }
    } else if (addr == 0x4014) {
        // OAM DMA Transfer
        m_dmaPage = data;
        m_dmaAddr = 0x00;
        m_dmaTransfer = true;
    } else if (addr == 0x4016) {
        if (m_controller1) m_controller1->write(data);
        if (m_controller2) m_controller2->write(data);
    } else if (addr >= 0x4000 && addr <= 0x4017) {
        if (m_apu) {
            m_apu->cpuWrite(addr, data);
        }
    }
}

void Bus::reset() {
    if (m_cpu) m_cpu->reset();
    if (m_ppu) m_ppu->reset();
    if (m_apu) m_apu->reset();
    m_systemClockCounter = 0;
}

void Bus::clock() {
    if (m_ppu) m_ppu->clock();

    // CPU runs at 1/3 speed of PPU
    if (m_systemClockCounter % 3 == 0) {
        if (m_dmaTransfer) {
            if (m_dmaDummy) {
                if (m_systemClockCounter % 2 == 1) {
                    m_dmaDummy = false;
                }
            } else {
                if (m_systemClockCounter % 2 == 0) {
                    m_dmaData = cpuRead((uint16_t(m_dmaPage) << 8) | m_dmaAddr);
                } else {
                    if (m_ppu) {
                        m_ppu->getOAMPtr()[m_dmaAddr] = m_dmaData;
                    }
                    m_dmaAddr++;
                    if (m_dmaAddr == 0x00) {
                        m_dmaTransfer = false;
                        m_dmaDummy = true;
                    }
                }
            }
        } else {
            if (m_cpu) m_cpu->clock();
        }
    }

    if (m_apu) m_apu->clock();

    if (m_ppu && m_ppu->nmi) {
        m_ppu->nmi = false;
        if (m_cpu) m_cpu->triggerNMI();
    }

    if (m_cartridge && m_cartridge->irqActive()) {
        m_cartridge->irqClear();
        if (m_cpu) m_cpu->irq();
    }

    m_systemClockCounter++;
}
