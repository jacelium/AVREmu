#pragma once

#include "memory.h"
#include "opset.h"
#include <iostream>

//typedef word 

class Emulator {
public:
	enum State {RUN, STOP, BREAK};
	enum Sreg {carry, zero, neg, overflow, sign, halfcarry, bit, interrupt};

	Emulator();
	Emulator(Memory * mem);
	~Emulator();

	// config
	void setMemory(Memory * mem);
	void setInterruptInterval(int interval);
	

	// run/inspect
	void step();
	void run();
	void setPC(int pc);
	State getState();


private:
		Memory * mem = nullptr;
		//Opset * op;
		int pc = 0;
		int cycles = 0;
		int interruptInterval = 10;
		int interruptCountdown = 0;
		State state;
		bool verbose = true;

		void handleInterrupts();
};