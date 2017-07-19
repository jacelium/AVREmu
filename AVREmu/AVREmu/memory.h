#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

class Memory {
private:
	std::vector<byte> memory;
	void memoryError(int address);
	enum MMIO {MMIO_OFFSET = 0x20, MMIO_START = 0, MMIO_END = 0x20};

public:
	byte readByte(int address);
	void writeByte(int address, byte data);
	word readWord(int address);
	void writeWord(int address, word data);

	void dumpInterpreted(int start, int count);
	std::vector<byte> dumpRaw(int start, int count);
	
	void setBit(int address, int bitnum);
	void clearBit(int address, int bitnum);
	int readBit(int address, int bitnum);

	void setRegister(int addr, byte val);
	void setRegister(int addr, word val);
	unsigned int getSize();

	Memory(int size);
};

