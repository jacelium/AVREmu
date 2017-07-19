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
	std::cerr << "Illegal memory access: " << std::hex << address << std::dec << std::endl;
}

void Memory::writeByte(int address, byte data) {
	try {
		this->memory.at(address) = data; 
		if ((address >= MMIO_START) && (address <= MMIO_END))  {
			this->memory.at(address + MMIO_OFFSET) = data; // mirror MMIO data
		}
	}
	catch (const std::out_of_range& e) { memoryError(address); }
}

byte Memory::readByte(int address) {
	try { return this->memory.at(address); }
	catch (const std::out_of_range& e) {
		memoryError(address);
		return 0x00;
	}
}

void Memory::writeWord(int address, word data) {
	writeByte(address + 1, data >> 8);
	writeByte(address, data & 0xFF);
}

word Memory::readWord(int address) {
	return (readByte(address + 1) << 8) | readByte(address);
}

void Memory::setBit(int address, int bitnum) {
	try { this->memory.at(address) |= (1 << bitnum); }
	catch (const std::out_of_range& e) { memoryError(address); }
}

void Memory::clearBit(int address, int bitnum) {
	try { this->memory.at(address) &= (~(1 << bitnum));	}
	catch (const std::out_of_range& e) { memoryError(address); }
}

int Memory::readBit(int address, int bitnum) {
	int bit = 0;
	try { if (this->memory.at(address) & (1 << bitnum)) bit = 1; }
	catch (const std::out_of_range& e) {
		memoryError(address); 
	}
	return bit;
}

std::vector<byte> Memory::dumpRaw(int start, int count) {
	std::vector<byte> results;
	for (auto result = this->memory.begin() + start; result < this->memory.begin() + start + count; result++) {
		results.push_back(*result);
	}
	return results;
}

void Memory::setRegister(int addr, byte val) {
	writeByte(addr, val);
}

void Memory::setRegister(int addr , word val) {
	writeByte(addr, val & 0x0f);
	writeByte(addr + 1, val >> 4);
}

unsigned int Memory::getSize() {
	return this->memory.size();
}