#pragma once

#include <string>
#include "EmulatorIF.h"
#include "memory.h"

class Emulator;

class Plugin {
public:
	void tick();
	virtual void generateState();
	virtual void applyState();
	void commit(unsigned int cycles);
	void uncommit();
	bool busy();
	void registerHost(Emulator * host, unsigned int id);
	void log(std::string msg);

protected: 
	Emulator * host;
	unsigned int busycount;
	unsigned int id;
};