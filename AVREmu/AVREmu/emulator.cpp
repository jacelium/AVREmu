#include "emulator.h"
#include <iostream>
#include <bitset>

Emulator::Emulator() {

}

Emulator::Emulator(Memory * mem) {
	this->mem = mem;
}

Emulator::~Emulator() {

}

void Emulator::step() {
	if (interruptCountdown <= 0) {
		interruptCountdown = interruptInterval;
		if (verbose) std::cout << "Checking interrupts." << std::endl;
		//handleInterrupts();
	}
	else {
		cycles++;
		interruptCountdown--;

		try {
			// retrieve
			unsigned char low = mem->readMemory(pc);
			unsigned char high = mem->readMemory(pc + 1);

			if (verbose) {
				std::cout << "Reading 0x" << std::hex << pc << std::dec;
				std::cout << ": H" << std::bitset<8>(high) << " L" << std::bitset<8>(low) << " Op " << int(high) << std::endl;
			}

			unsigned char d = 0x00, r = 0x00;

			//decode

			enum opcodes {
				NOP = 0, MOVW = 1, MULS = 2,
				
				ADD = 12, ADD2 = 13, ADD3 = 14, ADD4 = 15, ADC = 28, ADC2 = 29, ADC3 = 30, ADC4 = 31, 
				LSL = 12, LSL2 = 13, LSL3 = 14, LSL4 = 15, ROL = 28, ROL2 = 29, ROL3 = 30, ROL4 = 31, // If Rd = Rr 

				SBC = 8, SBC2 = 9, SBC3 = 10, SBC4 = 11, SUB = 24, SUB2 = 25, SUB3 = 26, SUB4 = 27,

				CPC = 4, CPC2 = 5, CPC3 = 6, CPC4 = 7,
				CP = 20, CP2 = 21, CP3 = 22, CP4 = 23,
				CPSE = 16, CPSE2 = 17, CPSE3 = 18, CPSE4 = 19,
				
				AND = 32, AND2 = 33, AND3 = 34, AND4 = 35,
				EOR = 36, EOR2 = 37, EOR3 = 38, EOR4 = 39,
				OR = 40, OR2 = 41, OR3 = 42, OR4 = 43,

				
				MOV = 44, MOV2 = 45, MOV3 = 46, MOV4 = 47,
				IJMP = 148 // H148 L9
			};

			switch (high) {
				// rdddddrrrr patterns
			case NOP:
				if (verbose) std::cout << "-> NOP" << std::endl;
				pc += 2;
				break;
			case MOV:
			case MOV2:
			case MOV3:
			case MOV4:
				d = (((high & (1 << 0)) << 4) | (low >> 4));
				r = (((high & (1 << 1)) << 3) | (low & 0x0f));
				if (verbose) std::cout << "-> MOV 0x" << std::hex << (int)r << " 0x" << (int)d << std::dec << std::endl;
				mem->writeMemory(d, mem->readMemory(r));
				pc += 2;
				break;
			case SUB:
			case SUB2:
				d = (((high & (1 << 0)) << 4) | (low >> 4));
				r = (((high & (1 << 1)) << 3) | (low & 0x0f));
				if (verbose) std::cout << "-> SUB 0x" << std::hex << (int)r << " 0x" << (int)d << std::dec << std::endl;
				mem->writeMemory(d, mem->readMemory(d) - mem->readMemory(r));
				pc += 2;
				break;
			case SBC:
			case SBC2:
				break;
			case ADD:
			case ADD2:
				d = (((high & (1 << 0)) << 4) | (low >> 4));
				r = (((high & (1 << 1)) << 3) | (low & 0x0f));
				if (d == r) {
					if (verbose) std::cout << "-> LSL 0x" << std::hex << (int)d << std::endl;;
					mem->writeMemory(d, mem->readMemory(d) << 1u);
					pc += 2;
				}
				else {
					if (verbose) std::cout << "-> ADD 0x" << std::hex << (int)r << " 0x" << (int)d << std::dec << std::endl;
					mem->writeMemory(d, mem->readMemory(d) + mem->readMemory(r));
					pc += 2;
				}
				break;
			case IJMP:
				pc = (mem->readMemory(Memory::ZH) << 8) | (mem->readMemory(Memory::ZL));
				if (verbose) std::cout << "-> IJMP " << pc << std::endl;
				break;
			default:
				throw "Invalid opcode.";
				break;
			}
		}
		catch (const char* except) {
			std::cout << except << std::endl;
		}
	}
}

void Emulator::run() {
	while (this->state == RUN) {
		this->step();
	}
}

Emulator::State Emulator::getState() {
	return Emulator::RUN;
}

void Emulator::setPC(int pc) {
	this->pc = pc;
}