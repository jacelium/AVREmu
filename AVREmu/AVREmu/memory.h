#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

struct MirroredRange {
	unsigned int start;
	unsigned int end;
	int offset;
};

class Memory {
private:
	std::vector<byte> memory;
	void memoryError(int address);

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

	std::vector<MirroredRange *> maps;
};

