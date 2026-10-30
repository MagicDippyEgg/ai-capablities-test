#include "APU.hpp"
#include <cmath>
#include <algorithm>

static const uint8_t DUTY_CYCLES[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0}, // 12.5%
    {0, 1, 1, 0, 0, 0, 0, 0}, // 25%
    {0, 1, 1, 1, 1, 0, 0, 0}, // 50%
    {1, 0, 0, 1, 1, 1, 1, 1}  // 25% negated
};

static const uint8_t LENGTH_TABLE[32] = {
    10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
    12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

static const uint16_t NOISE_PERIOD_TABLE[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

static const uint8_t TRIANGLE_SEQUENCE[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

APU::APU() = default;
APU::~APU() = default;

void APU::cpuWrite(uint16_t addr, uint8_t data) {
    switch (addr) {
    // Pulse 1
    case 0x4000:
        m_pulse1.duty = (data >> 6) & 0x03;
        m_pulse1.volume = data & 0x0F;
        m_pulse1.constantVolume = (data & 0x10) != 0;
        break;
    case 0x4002:
        m_pulse1.timerValue = (m_pulse1.timerValue & 0x0700) | data;
        break;
    case 0x4003:
        m_pulse1.timerValue = (m_pulse1.timerValue & 0x00FF) | ((uint16_t)(data & 0x07) << 8);
        m_pulse1.lengthCounter = LENGTH_TABLE[(data >> 3) & 0x1F];
        m_pulse1.sequencePos = 0;
        break;

    // Pulse 2
    case 0x4004:
        m_pulse2.duty = (data >> 6) & 0x03;
        m_pulse2.volume = data & 0x0F;
        m_pulse2.constantVolume = (data & 0x10) != 0;
        break;
    case 0x4006:
        m_pulse2.timerValue = (m_pulse2.timerValue & 0x0700) | data;
        break;
    case 0x4007:
        m_pulse2.timerValue = (m_pulse2.timerValue & 0x00FF) | ((uint16_t)(data & 0x07) << 8);
        m_pulse2.lengthCounter = LENGTH_TABLE[(data >> 3) & 0x1F];
        m_pulse2.sequencePos = 0;
        break;

    // Triangle
    case 0x400A:
        m_triangle.timerValue = (m_triangle.timerValue & 0x0700) | data;
        break;
    case 0x400B:
        m_triangle.timerValue = (m_triangle.timerValue & 0x00FF) | ((uint16_t)(data & 0x07) << 8);
        m_triangle.lengthCounter = LENGTH_TABLE[(data >> 3) & 0x1F];
        break;

    // Noise
    case 0x400C:
        m_noise.volume = data & 0x0F;
        break;
    case 0x400E:
        m_noise.mode = (data & 0x80) != 0;
        m_noise.timerValue = NOISE_PERIOD_TABLE[data & 0x0F];
        break;
    case 0x400F:
        m_noise.lengthCounter = LENGTH_TABLE[(data >> 3) & 0x1F];
        break;

    // Control
    case 0x4015:
        m_pulse1.enabled = (data & 0x01) != 0;
        if (!m_pulse1.enabled) m_pulse1.lengthCounter = 0;

        m_pulse2.enabled = (data & 0x02) != 0;
        if (!m_pulse2.enabled) m_pulse2.lengthCounter = 0;

        m_triangle.enabled = (data & 0x04) != 0;
        if (!m_triangle.enabled) m_triangle.lengthCounter = 0;

        m_noise.enabled = (data & 0x08) != 0;
        if (!m_noise.enabled) m_noise.lengthCounter = 0;
        break;
    }
}

uint8_t APU::cpuRead(uint16_t addr) {
    if (addr == 0x4015) {
        uint8_t data = 0x00;
        if (m_pulse1.lengthCounter > 0) data |= 0x01;
        if (m_pulse2.lengthCounter > 0) data |= 0x02;
        if (m_triangle.lengthCounter > 0) data |= 0x04;
        if (m_noise.lengthCounter > 0) data |= 0x08;
        return data;
    }
    return 0x00;
}

void APU::reset() {
    m_clockCounter = 0;
    m_frameSequencerStep = 0;
    m_pulse1 = {};
    m_pulse2 = {};
    m_triangle = {};
    m_noise = {};
    m_noise.shiftRegister = 1;
}

void APU::clock() {
    m_clockCounter++;

    // Pulse 1 timer
    if (m_clockCounter % 2 == 0) {
        if (m_pulse1.timer == 0) {
            m_pulse1.timer = m_pulse1.timerValue;
            m_pulse1.sequencePos = (m_pulse1.sequencePos + 1) & 0x07;
        } else {
            m_pulse1.timer--;
        }

        if (m_pulse2.timer == 0) {
            m_pulse2.timer = m_pulse2.timerValue;
            m_pulse2.sequencePos = (m_pulse2.sequencePos + 1) & 0x07;
        } else {
            m_pulse2.timer--;
        }

        if (m_noise.timer == 0) {
            m_noise.timer = m_noise.timerValue;
            uint16_t shift = m_noise.mode ? 6 : 1;
            uint16_t bit = (m_noise.shiftRegister ^ (m_noise.shiftRegister >> shift)) & 0x01;
            m_noise.shiftRegister = (m_noise.shiftRegister >> 1) | (bit << 14);
        } else {
            m_noise.timer--;
        }
    }

    // Triangle timer
    if (m_triangle.timer == 0) {
        m_triangle.timer = m_triangle.timerValue;
        if (m_triangle.lengthCounter > 0) {
            m_triangle.sequencePos = (m_triangle.sequencePos + 1) & 0x1F;
        }
    } else {
        m_triangle.timer--;
    }

    // Frame Sequencer (~240 Hz / ~7458 CPU clocks)
    if (m_clockCounter % 7458 == 0) {
        m_frameSequencerStep = (m_frameSequencerStep + 1) % 4;

        if (m_frameSequencerStep == 1 || m_frameSequencerStep == 3) {
            // Clock length counters
            if (m_pulse1.lengthCounter > 0) m_pulse1.lengthCounter--;
            if (m_pulse2.lengthCounter > 0) m_pulse2.lengthCounter--;
            if (m_triangle.lengthCounter > 0) m_triangle.lengthCounter--;
            if (m_noise.lengthCounter > 0) m_noise.lengthCounter--;
        }
    }
}

float APU::getOutputSample() const {
    float p1 = 0.0f;
    if (m_pulse1.enabled && m_pulse1.lengthCounter > 0 && m_pulse1.timerValue >= 8) {
        p1 = DUTY_CYCLES[m_pulse1.duty][m_pulse1.sequencePos] * m_pulse1.volume;
    }

    float p2 = 0.0f;
    if (m_pulse2.enabled && m_pulse2.lengthCounter > 0 && m_pulse2.timerValue >= 8) {
        p2 = DUTY_CYCLES[m_pulse2.duty][m_pulse2.sequencePos] * m_pulse2.volume;
    }

    float tri = 0.0f;
    if (m_triangle.enabled && m_triangle.lengthCounter > 0) {
        tri = TRIANGLE_SEQUENCE[m_triangle.sequencePos];
    }

    float noise = 0.0f;
    if (m_noise.enabled && m_noise.lengthCounter > 0 && !(m_noise.shiftRegister & 0x01)) {
        noise = m_noise.volume;
    }

    float pulseOut = (p1 + p2 > 0) ? (95.88f / ((8128.0f / (p1 + p2)) + 100.0f)) : 0.0f;
    float tndOut = (tri + noise > 0) ? (159.79f / ((1.0f / (tri / 8227.0f + noise / 12241.0f)) + 100.0f)) : 0.0f;

    return std::clamp(pulseOut + tndOut, 0.0f, 1.0f);
}
