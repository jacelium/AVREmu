#include "memory.h"
#include "emulator.h"
#include <iostream>
#include <bitset>
#include <vector>
#include <stdexcept>

void main() {
	Memory test(512);
	
	test.writeMemory(Memory::r3, 0x0C); // 00001100 // add r0 to r1 
	test.writeMemory(Memory::r2, 0x10); // 00010001

	test.writeMemory(Memory::r5, 0x2c); // 00101100 // mov r1 to r14
	test.writeMemory(Memory::r4, 0xe1); // 11100001

	test.writeMemory(Memory::r7, 0x0C); // add r0 to r1
	test.writeMemory(Memory::r6, 0x10);
	
	test.writeMemory(Memory::r9, 0x2c); // 00101100 // mov r1 to r15
	test.writeMemory(Memory::r8, 0xf1); // 11110001

	test.writeMemory(Memory::r11, 0x94); // 10010100 // IJMP (jump to address in Z)
	test.writeMemory(Memory::r10, 0x09); // 00001001

	test.writeMemory(Memory::r1, 'b'); // source data, in r0
	test.writeMemory(Memory::r0, 1); // destination, r1

	test.setRegister(Memory::ZH, 0x00);
	test.setRegister(Memory::ZL, 0x02);

	//test.dumpRaw(0, 16);

	Emulator testEmu(&test);
	testEmu.setPC(0x02);

	int x = 50;

	while (x > 0) {
		testEmu.step();

		x--;
	}
	test.dumpRaw(0, 16);
	std::cout << std::bitset<8>(test.readMemory(0x0e)) << " " << test.readMemory(0x0e) << std::endl;
	std::cout << std::bitset<8>(test.readMemory(0x0f)) << " " << test.readMemory(0x0f) << std::endl;
	system("pause");
}