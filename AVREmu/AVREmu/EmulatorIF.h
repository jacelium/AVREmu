#pragma once

#include "memory.h"
#include "PluginIF.h"

class EmulatorIF {
public:
	virtual enum State { RUN, STOP, BREAK, STATESIZE };
	enum Config { VERBOSE, CONFIGSIZE };

	EmulatorIF();
	EmulatorIF(Memory * mem);
	~EmulatorIF();

	// config
	virtual void setMemory(Memory * mem);
	virtual void setInterruptInterval(int interval);
	virtual void configure(Config conf, bool state);
	virtual bool getConfig(Config conf);
	virtual void setState(State newState);
	virtual unsigned int clockSubscribe(PluginIF * client);
	virtual void setInstructionWidth(unsigned int instructionWidth);

	// run/inspect
	virtual void step();
	virtual void run();
	virtual void setPC(int pc);
	State getState();
	virtual void tick();

	virtual void writeByte(int address, byte data);
	virtual void writeWord(int address, word data);
	virtual byte readByte(int address);
	virtual word readWord(int address);

	virtual void loadProgram(const char * progFile);

protected:
	virtual void handleInterrupts();
};