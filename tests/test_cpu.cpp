#include <iostream>
#include <cassert>
#include "CPU.hpp"
#include "Bus.hpp"

void testLDA_Immediate() {
    Bus bus;
    CPU cpu;
    bus.connectCPU(&cpu);
    cpu.connectBus(&bus);

    bus.ram[0x0000] = 0xA9; // LDA #$42
    bus.ram[0x0001] = 0x42;

    cpu.pc = 0x0000;
    cpu.clock(); // execute LDA #$42
    while (!cpu.complete()) cpu.clock();

    assert(cpu.a == 0x42);
    assert((cpu.status & CPU::Z) == 0);
    assert((cpu.status & CPU::N) == 0);
    std::cout << "testLDA_Immediate passed!" << std::endl;
}

void testADC_OverflowAndCarry() {
    Bus bus;
    CPU cpu;
    bus.connectCPU(&cpu);
    cpu.connectBus(&bus);

    cpu.a = 0x50; // +80
    bus.ram[0x0000] = 0x69; // ADC #$50 (+80)
    bus.ram[0x0001] = 0x50;

    cpu.pc = 0x0000;
    cpu.clock();
    while (!cpu.complete()) cpu.clock();

    assert(cpu.a == 0xA0); // -96 in 2's complement
    assert((cpu.status & CPU::V) != 0); // Overflow flag set
    assert((cpu.status & CPU::N) != 0); // Negative flag set
    std::cout << "testADC_OverflowAndCarry passed!" << std::endl;
}

void testStackAndSubroutine() {
    Bus bus;
    CPU cpu;
    bus.connectCPU(&cpu);
    cpu.connectBus(&bus);

    // JSR $0005 at $0000
    bus.ram[0x0000] = 0x20;
    bus.ram[0x0001] = 0x05;
    bus.ram[0x0002] = 0x00;

    // RTS at $0005
    bus.ram[0x0005] = 0x60;

    cpu.pc = 0x0000;
    cpu.stp = 0xFD;

    // Execute JSR
    cpu.clock();
    while (!cpu.complete()) cpu.clock();
    assert(cpu.pc == 0x0005);

    // Execute RTS
    cpu.clock();
    while (!cpu.complete()) cpu.clock();
    assert(cpu.pc == 0x0003);

    std::cout << "testStackAndSubroutine passed!" << std::endl;
}

int main() {
    testLDA_Immediate();
    testADC_OverflowAndCarry();
    testStackAndSubroutine();
    std::cout << "All CPU unit tests passed!" << std::endl;
    return 0;
}
