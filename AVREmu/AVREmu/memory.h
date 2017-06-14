#pragma once

#include <iostream>
#include <vector>

class Memory {
private:
	std::vector<unsigned char> memory;
	void memoryError(int address);

public:
	enum reg { // named access to registers/register pairs
		r0 = 0x00, r1 = 0x01, r2 = 0x02, r3 = 0x03, r4 = 0x04, r5 = 0x05, r6 = 0x06, r7 = 0x07,
		r8 = 0x08, r9 = 0x09, r10 = 0x0a, r11 = 0x0b, r12 = 0x0c, r13 = 0x0d, r14 = 0x0e, r15 = 0x0f,
		r16 = 0x10, r17 = 0x11, r18 = 0x12, r19 = 0x13, r20 = 0x14, r21 = 0x15, r22 = 0x16, r23 = 0x17,
		r24 = 0x18, r25 = 0x19, r26 = 0x1a, r27 = 0x1b, r28 = 0x1c, r29 = 0x1d, r30 = 0x1e, r31 = 0x1f,
		XL = 0x1a, XH = 0x1b, rX = 0x1a, YL = 0x1c, YH = 0x1d, rY = 0x1c, ZL = 0x1e, ZH = 0x1f, rZ = 0x1e
	};

	unsigned char readMemory(int address);
	void writeMemory(int address, unsigned char data);

	void dumpInterpreted(int start, int count);
	void dumpRaw(int start, int count, int cols = 8);
	
	void setBit(int address, int bitnum);
	void clearBit(int address, int bitnum);

	void setRegister(reg r, unsigned char val);

	Memory(int size);
};

