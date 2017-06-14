/* TODO:
	Aliasing MMIO (0x20 offsets)
	More elegant invalid memory access reporting
*/

#include "memory.h"
#include <iostream>
#include <bitset>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>

Memory::Memory(int size) {
	this->memory.resize(size);
}

void Memory::memoryError(int address) {
	std::cerr << "Illegal memory access: " << address << std::endl;
}

void Memory::writeMemory(int address, unsigned char data) {
	try {
		this->memory.at(address) = data;
	}
	catch (const std::out_of_range& e) { memoryError(address); }
}

void Memory::setBit(int address, int bitnum) {
	try {
	this->memory[address] |= (1 << bitnum);
	}
	catch (const std::out_of_range& e) { memoryError(address); }
}

void Memory::clearBit(int address, int bitnum) {
	try {
		this->memory.at(address) &= (~(1 << bitnum));
	}
	catch (const std::out_of_range& e) { memoryError(address); }
}

unsigned char Memory::readMemory(int address) {
	try {
		return this->memory.at(address);
	}
	catch (const std::out_of_range& e) {
		memoryError(address); 
		return 0x00;
	}
}

void Memory::dumpInterpreted(int start, int count) {
	std::cout << "Dumping " << count << " bytes starting at " << start << std::endl << std::endl;

	std::cout << "Raw:\t Char:\t :Int:" << std::endl;

	for (int i = start; i < start + count; i++) {
		std::bitset<8> x(readMemory(i));
		std::cout << x << "\t" << readMemory(i) << "\t" << (int)readMemory(i) << std::endl;
	}
	std::cout << std::endl;
}

void Memory::dumpRaw(int start, int count, int cols /*8*/) {
	std::cout << "Dumping " << count << " bytes starting at 0x" << start << std::endl;
	for (int offset = 0; offset < count; offset++) {
		if (offset % cols == 0) {
			//if (offset == 0) std::cout << std::endl << std::hex << start + offset << std::dec;
			std::cout << std::endl;
		}
		std::bitset<8> x(readMemory(start + offset));
		std::cout << x << " ";
	}
	std::cout << std::endl << std::endl;
}

void Memory::setRegister(reg r, unsigned char val) {
	writeMemory(r, val);
}