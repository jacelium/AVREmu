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
	std::string getName();

protected: 
	Emulator * host;
	unsigned int busycount;
	unsigned int id;
	virtual std::string name();
	bool postCommit;
};