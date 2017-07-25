#pragma once

#include "EmulatorIF.h"
#include "opset.h"
#include "plugin.h"
#include <iostream>
#include <cstdint>

class Emulator {
public:
	enum State {RUN, STOP, BREAK, STATESIZE};
	enum Config {VERBOSE, CONFIGSIZE};

	Emulator();
	Emulator(Memory * mem);
	~Emulator();

	// config
	void setMemory(Memory * mem);
	void setInterruptInterval(int interval);
	void configure(Config conf, bool state);
	bool getConfig(Config conf);
	void setState(State newState);
	unsigned int clockSubscribe(Plugin * client);
	void setInstructionWidth(unsigned int instructionWidth);
	unsigned int movePC(int offset = 1);

	// run/inspect
	void step();
	void run();
	void setPC(int pc);
	State getState();
	void tick();

	void writeByte(int address, byte data);
	void writeWord(int address, word data);
	byte readByte(int address); 
	word readWord(int address);
	void setBit(int address, int bitnum);
	void clearBit(int address, int bitnum);
	int readBit(int address, int bitnum);

	void loadProgram(const char * progFile, int offset = 0);

	unsigned int clockRate;

	Memory * mem = nullptr;

	unsigned int pc = 0; // Program counter
	unsigned int cycles = 0; // Number of elapsed cycles
	unsigned int instructionSize = 2; // Width of instruction in bytes, 2 for AVR

protected:
		void handleInterrupts();
		int interruptInterval = 10; // Interval between interrupt checks
		State state;
		std::vector<bool> config;
		std::vector<Plugin *> subscribers;
};