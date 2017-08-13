#pragma once

#include "EmulatorIF.h"
#include "opset.h"
#include "plugin.h"
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>

class Emulator {
public:
	enum State {RUN, STOP, BREAK, STATESIZE};
	enum Config {VERBOSE, CONFIGSIZE};
	enum Loglevel {FATAL, WARN, INFO, ALL, LOGLEVELSIZE};

	enum Memtype {SRAM, PROGRAM, EEPROM, MEMSIZE};

	Emulator();
	~Emulator();

	// config
	void setMemory(Memory * mem, Memtype memory = SRAM);
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
	void log(std::string logString, Loglevel priority = INFO);

	int logLevel = 3;

	void writeByte(int address, byte data, Memtype memory = SRAM);
	void writeWord(int address, word data, Memtype memory = SRAM);
	byte readByte(int address, Memtype memory = SRAM);
	word readWord(int address, Memtype memory = SRAM);
	void setBit(int address, int bitnum, Memtype memory = SRAM);
	void clearBit(int address, int bitnum, Memtype memory = SRAM);
	int readBit(int address, int bitnum, Memtype memory = SRAM);

	void loadProgram(const char * progFile);

	unsigned int clockRate;

	std::vector<Memory *> mem;

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