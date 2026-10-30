cmake_minimum_required(VERSION 3.16)
project(NESEmulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

include_directories(include ${SDL2_INCLUDE_DIRS})

add_library(nes_core
    src/Cartridge.cpp
    src/Mapper.cpp
    src/Mapper0.cpp
    src/CPU.cpp
    src/PPU.cpp
    src/APU.cpp
    src/Controller.cpp
    src/Bus.cpp
    src/NES.cpp
)

target_link_libraries(nes_core ${SDL2_LIBRARIES})

add_executable(nes_emulator src/main.cpp)
target_link_libraries(nes_emulator nes_core ${SDL2_LIBRARIES})

enable_testing()

add_executable(test_cpu tests/test_cpu.cpp)
target_link_libraries(test_cpu nes_core)
add_test(NAME TestCPU COMMAND test_cpu)

add_executable(test_cartridge tests/test_cartridge.cpp)
target_link_libraries(test_cartridge nes_core)
add_test(NAME TestCartridge COMMAND test_cartridge)

add_executable(test_ppu tests/test_ppu.cpp)
target_link_libraries(test_ppu nes_core)
add_test(NAME TestPPU COMMAND test_ppu)

add_executable(test_controller tests/test_controller.cpp)
target_link_libraries(test_controller nes_core)
add_test(NAME TestController COMMAND test_controller)

add_executable(test_integration tests/test_integration.cpp)
target_link_libraries(test_integration nes_core)
add_test(NAME TestIntegration COMMAND test_integration)
