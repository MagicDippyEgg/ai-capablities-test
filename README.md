# NES Emulator From Scratch

A fully functional Nintendo Entertainment System (NES) emulator built from scratch in modern C++ (C++17) using SDL2 for windowing, rendering, and audio output.

The primary target for initial compatibility is **Super Mario Bros. (NTSC, Mapper 0 / NROM)**.

---

## Features

- **CPU Core (Ricoh 2A03 / 6502)**: Complete implementation of all official 6502 opcodes, 13 addressing modes, status flags, stack operations, interrupts (RESET, NMI, IRQ), and cycle timing.
- **Picture Processing Unit (PPU 2C02)**: Complete background rendering, 8x8 & 8x16 sprite rendering, sprite 0 hit detection, sprite evaluation, Loopy scrolling, palette table mapping, VBlank & NMI timing. Native 256x240 resolution rendered via integer scaling.
- **Audio Processing Unit (APU)**: Pulse 1, Pulse 2, Triangle, and Noise channel synthesis with non-linear audio output mixing.
- **Cartridge & Mapper Architecture**:
  - iNES and NES 2.0 header parsing.
  - **Mapper 0 (NROM)**: 16KB/32KB PRG ROM, 8KB CHR ROM/RAM.
  - **Mapper 1 (MMC1)**: 5-bit serial shift register, 16KB/32KB PRG bank switching, 4KB/8KB CHR bank switching, dynamic mirroring, PRG RAM support ($6000-$7FFF).
  - **Mapper 2 (UxROM)**: Switchable 16KB PRG ROM bank at $8000, fixed last 16KB bank at $C000.
  - **Mapper 3 (CNROM)**: Fixed PRG ROM, switchable 8KB CHR bank.
  - **Mapper 4 (MMC3)**: 8KB PRG banking, 1KB/2KB CHR banking, PRG/CHR inversion modes, dynamic mirroring, scanline IRQ counter.
- **Input / Controls**: NES standard controller emulation ($4016/$4017 strobe & shift register logic) with keyboard mapping.
- **Emulator GUI**: SDL2 front-end with integer scaling options (1x, 2x, 3x, 4x), pause/resume (`P`), reset (`R`), window resizing, and frame limiter (60 FPS target).

---

## Prerequisites & Dependencies (Linux)

To build and run the emulator on Linux, ensure the following packages are installed:

- **GCC / G++** (supporting C++17) or **Clang**
- **CMake** (v3.16 or higher)
- **Make** / **Ninja**
- **libsdl2-dev**
- **pkg-config**

### Ubuntu / Debian Installation
```bash
sudo apt update
sudo apt install -y build-essential cmake libsdl2-dev pkg-config
```

---

## Building the Emulator

1. Clone the repository:
```bash
git clone https://github.com/your-username/nes-emulator.git
cd nes-emulator
```

2. Configure and build in Release mode:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The compiled binary will be located at `build/nes_emulator`.

---

## Running Unit & Integration Tests

Run the test suite using CTest:
```bash
ctest --test-dir build --output-on-failure
```

The test suite validates:
- CPU addressing modes, opcodes, stack, and branch behavior (`test_cpu`).
- iNES cartridge parsing, Mapper 0, Mapper 1 (MMC1), Mapper 2 (UxROM), Mapper 3 (CNROM), Mapper 4 (MMC3), and PRG RAM mapping (`test_cartridge`).
- PPU register reads/writes, Loopy scroll, and VBlank/NMI triggers (`test_ppu`).
- Controller strobe and shift register mechanics (`test_controller`).
- Integration test executing 120 full frames of emulation (`test_integration`).

---

## Controls & Usage

### Launching a ROM

```bash
./build/nes_emulator path/to/game.nes
```

Optional flags:
- `--scale <1-8>`: Set integer scaling factor (default: 3, resulting in 768x720 window).

### Keyboard Controls

| NES Button | Keyboard Key |
|------------|--------------|
| **D-Pad Up** | Up Arrow |
| **D-Pad Down** | Down Arrow |
| **D-Pad Left** | Left Arrow |
| **D-Pad Right** | Right Arrow |
| **A** | `Z` |
| **B** | `X` |
| **Select** | Left Shift / Right Shift |
| **Start** | Enter |

### Emulator Shortcuts

| Action | Key |
|--------|-----|
| **Pause / Resume** | `P` |
| **Reset** | `R` |
| **Scale 1x (256x240)** | `1` |
| **Scale 2x (512x480)** | `2` |
| **Scale 3x (768x720)** | `3` |
| **Scale 4x (1024x960)** | `4` |

---

## GitHub Actions Automated CI Build

An automated GitHub Actions workflow is provided in `.github/workflows/build.yml`.

- **Triggers**: Pushes and Pull Requests targeting the `main` branch.
- **Environment**: `ubuntu-24.04`.
- **Workflow steps**:
  1. Installs `build-essential`, `cmake`, `libsdl2-dev`, `pkg-config`.
  2. Builds the emulator in `Release` mode.
  3. Executes `ctest` to ensure all tests pass.
  4. Packages the compiled `nes_emulator` executable and `README.md`.
  5. Uploads the output artifact as `nes-emulator-linux-x86_64`.

---

## Architecture & Code Structure

```
├── include/
│   ├── APU.hpp          # Audio processing unit (Pulse 1, 2, Triangle, Noise)
│   ├── Bus.hpp          # System interconnect & memory mapping
│   ├── Cartridge.hpp    # iNES file parser, PRG RAM & memory storage
│   ├── Controller.hpp   # Controller strobe and shift register logic
│   ├── CPU.hpp          # Ricoh 2A03 6502 CPU implementation
│   ├── Mapper.hpp       # Mapper abstract base class
│   ├── Mapper0.hpp      # NROM (Mapper 0) implementation
│   ├── Mapper1.hpp      # MMC1 (Mapper 1) implementation
│   ├── Mapper2.hpp      # UxROM (Mapper 2) implementation
│   ├── Mapper3.hpp      # CNROM (Mapper 3) implementation
│   ├── Mapper4.hpp      # MMC3 (Mapper 4) implementation
│   ├── NES.hpp          # High-level system container
│   └── PPU.hpp          # Picture Processing Unit 2C02 implementation
├── src/                 # System implementations & main GUI loop
├── tests/               # Unit and integration tests
├── .github/workflows/   # GitHub Actions CI workflow
├── CMakeLists.txt       # Build system configuration
└── README.md            # Documentation
```

---

## Known Limitations

- Audio synthesis provides basic Pulse/Triangle/Noise emulation; DMC channel is currently stubbed.
