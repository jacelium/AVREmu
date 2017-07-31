#include "memory.h"
#include "emulator.h"
#include <iostream>
#include <bitset>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include "AVRInstructionSet.h"
#include <fstream>

typedef std::pair<int, std::string> test_item;

word make_add(byte r, byte d) {
	return 0x0000;
} 

void dumpRaw(Memory * mem, int start, int count) {
	std::vector<byte> results = mem->dumpRaw(start, count);
	std::cout << "[Host] Dumping " << count << " bytes starting at 0x" << start << ":" << std::endl;
	int col = 0, cols = 8;

	for (auto i = results.begin(); i != results.end(); i++) {
		if (col++ % cols == 0) { std::cout << std::endl << "\t"; } // line breaks

		std::bitset<8> x(*i);
		std::cout << x << " ";
	}
	std::cout << std::endl << std::endl;
}

class TestPlugin : public Plugin {
	void generateState() {
			std::cout << "Plugin " << id << " generating new state." << std::endl;
			commit(2);
	}
	void applyState() {
			std::cout << "Plugin " << id << " applying new state." << std::endl;
	}
};

class IOPort : public Plugin {
public:
	std::string name;
	word addr;

	IOPort(std::string name, word addr) {
		this->name = name;
		this->addr = addr;
	}

	void generateState() {
	}
	void applyState() {
		std::cout << "[" << name << "] " << std::bitset<8>(host->readByte(addr)) << std::endl;
	}
};

void loadProgramFromVector(std::vector<int> prog, Memory * mem) {
	int address = 0x00;

	auto end = prog.end();
	for (auto i = prog.begin(); i != prog.end(); i++) {
		mem->writeWord(address, *i);
		address += 2;
	}
}

void main() {
	Memory mem(0xFFFF);
	Memory sram(0x8FF);
	Memory eeprom(0x400);

	MirroredRange range{ 0x0, 0x20, 0x20 };
	MirroredRange range2{ 0x20, 0x40, 0x40 };

	AVRInstructionSet instructionSet;
	TestPlugin testPlugin;
	IOPort portB("PortB", 0x25);

	Emulator emulator;
	emulator.setMemory(&sram);
	emulator.setMemory(&mem, Emulator::PROGRAM);
	emulator.setMemory(&eeprom, Emulator::EEPROM);
	emulator.configure(Emulator::VERBOSE, true);

	emulator.clockSubscribe(&instructionSet);
	//emulator.clockSubscribe(&testPlugin);
	//emulator.clockSubscribe(&portB);

	int testCase = 6;

	if (testCase == 0) {
		// ADD, MOV, IJMP

		//mem.writeByte(AVRInstructionSet::r1, 'b'); // destination, in r1
		//mem.writeByte(AVRInstructionSet::r0, 1); // source data, r0

		mem.writeWord(AVRInstructionSet::r0, 0x6201);

		// 0000110000010000 - add r0 to r1
		mem.writeWord(AVRInstructionSet::r2, 0x0C10);

		// 1110000100101100 - mov r1 to r14
		mem.writeWord(AVRInstructionSet::r4, 0x2CE1);

		// 0000110000010001 - add r0 to r1
		mem.writeWord(AVRInstructionSet::r6, 0x0C10);

		// 0010110011110001 - mov r1 to r15
		mem.writeWord(AVRInstructionSet::r8, 0x2CF1);

		// 1001010 000001001 // IJMP (jump to address in Z)
		mem.writeWord(AVRInstructionSet::r10, 0x9409);

		mem.writeWord(AVRInstructionSet::r14, 0xFFFF);

		byte ZH = 0x00, ZL = 0x02;
		word addr = 0x0002; 

		mem.writeWord(AVRInstructionSet::rZ, addr); // JMP address; 0x02

		dumpRaw(&mem, 0, 16);

		emulator.setPC(0x02);
		int x = 32;
		for (int i = 0; i < x; i++) emulator.step();
		//emulator.run();

		dumpRaw(&mem, 0, 16);
		std::cout << std::bitset<8>(mem.readByte(0x0e)) << " " << mem.readByte(0x0e) << std::endl;
		std::cout << std::bitset<8>(mem.readByte(0x0f)) << " " << mem.readByte(0x0f) << std::endl;
	}
	else if (testCase == 1) {
		// ADD, MOV, IJMP from file

		std::cout <<
			"Test program: Writes 1 to 0x00, 'c' to 0x01 and 0x0002 to" << std::endl <<
			"              the Z register, sets PC to 0x02, then steps" << std::endl <<
			"              through the following loop for 15 cycles:" << std::endl << std::endl <<
			"                  ADD 0x00, 0x01" << std::endl <<
			"                  MOV 0x01, 0x0E" << std::endl <<
			"                  ADD 0x00, 0x01" << std::endl <<
			"                  MOV 0x01, 0x0F" << std::endl <<
			"                  IJMP Z" << std::endl << std::endl;

		emulator.loadProgram(".\\program.hex");

		dumpRaw(&mem, 0, 16);

		emulator.setPC(0x02); // ( * instructionWidth)

		int x = 15;
		for (int i = 0; i < x; i++) {

			emulator.step();
		}

		dumpRaw(&mem, 0, 16);
		std::cout << "0x0E: " << std::bitset<8>(mem.readByte(0x0e)) << " " << mem.readByte(0x0e) << std::endl;
		std::cout << "0x0F: " << std::bitset<8>(mem.readByte(0x0f)) << " " << mem.readByte(0x0f) << std::endl;
	}
	else if (testCase == 2) {
		//STS, LDS

		mem.writeByte(AVRInstructionSet::r0, 0xbb); // payload
		mem.writeWord(AVRInstructionSet::rZ, 0xF0); // STS address in Z
		mem.writeWord(AVRInstructionSet::rY, 0xF0); // LDS address in Y

		//100100sdddddyXXX

		//1001001000000000 - STS r0 -> i
		mem.writeWord(AVRInstructionSet::r2, 0x9200);
		mem.writeWord(AVRInstructionSet::r4, AVRInstructionSet::r12);

		//1001000000010000 - LDS i -> r1
		mem.writeWord(AVRInstructionSet::r6, 0x9010);
		mem.writeWord(AVRInstructionSet::r8, AVRInstructionSet::r12);

		mem.writeWord(AVRInstructionSet::r10, 0xFFFF);

		dumpRaw(&mem, 0, 16);
		dumpRaw(&mem, 0xF0, 1);

		emulator.setPC(0x02);
		emulator.run();

		dumpRaw(&mem, 0, 16);
		std::cout << "0x01: " << " " << mem.readByte(0x01) << std::endl;
		std::cout << "r12: " << " " << mem.readByte(AVRInstructionSet::r12) << std::endl;
		std::cout << mem.readBit(0x0e, 3) << mem.readBit(0x0e, 2) << mem.readBit(0x0e, 1) << mem.readBit(0x0e, 0) << std::endl;
	}
	else if (testCase == 3) {
		mem.writeWord(AVRInstructionSet::r0, 0x9408);
		mem.writeWord(AVRInstructionSet::r2, 0x9488);

		dumpRaw(&mem, 0, 4);
		dumpRaw(&mem, 0x5F, 1);

		emulator.setPC(0x00);
		emulator.tick();
		emulator.tick();
		emulator.tick();
		emulator.tick();


		dumpRaw(&mem, 0x5F, 1);
		emulator.tick();
		dumpRaw(&mem, 0x5F, 1);

	}
	else if (testCase == 4) {
		mem.writeWord(0x00, 0xFFFF);
		emulator.run();
		system("pause");
	}
	else if (testCase == 5) {
		
		//mem.setBit(0x5F, 2);

		mem.writeWord(0x00, 0xF40A); // BRNE 1
		mem.writeWord(0x02, 0x9598); // BREAK
		mem.writeWord(0x04, 0x9598); // BREAK
		emulator.run();

		system("pause");
	}
	else if (testCase == 6) {

		std::vector<int> prog;

		prog.push_back(0x12c0);
		prog.push_back(0x19c0);
		prog.push_back(0x18c0);
		prog.push_back(0x17c0);
		prog.push_back(0x16c0);
		prog.push_back(0x15c0);
		prog.push_back(0x14c0);
		prog.push_back(0x13c0);
		prog.push_back(0x12c0);
		prog.push_back(0x11c0);
		prog.push_back(0x10c0);
		prog.push_back(0x0fc0);
		prog.push_back(0x0ec0);
		prog.push_back(0x0dc0);
		prog.push_back(0x0cc0);
		prog.push_back(0x0bc0);
		prog.push_back(0x0ac0);
		prog.push_back(0x09c0);
		prog.push_back(0x08c0);
		prog.push_back(0x1124);
		prog.push_back(0x1fbe);
		prog.push_back(0xcfe5);
		prog.push_back(0xd4e0);
		prog.push_back(0xdebf);
		prog.push_back(0xcdbf);
		prog.push_back(0x02d0);
		prog.push_back(0x02c0);
		prog.push_back(0xe4cf);
		prog.push_back(0x0895);
		prog.push_back(0xf894);
		prog.push_back(0xffcf);

		//loadProgramFromVector(prog, &mem);
		emulator.loadProgram(".\\test2.hex");


		dumpRaw(&mem, 0x00, 64);
		dumpRaw(&mem, 0x3e, 2);
		emulator.run();
		/*for (int i = 0; i < 4; i++) { 
			emulator.step(); 
		}*/
	}
	else if (testCase == 7) {
		emulator.loadProgram(".\\test.hex");

		dumpRaw(&mem, 0x00, 64);
		dumpRaw(&mem, 0x3e, 2);
		emulator.run();
	}
	else
	{
		std::vector<std::string> dividers;

		dividers.push_back("==== LDST_INDIRECT (11111100) family (indirect ST/LD, LPM, POP/PUSH): ====");
		dividers.push_back("==== MUL_ETC family: (MOVW, MULS...) ====");
		dividers.push_back("==== MISC family: (RET/SLEEP/BREAK etc) ====");
		dividers.push_back("==== UNARY family: (COM, NEG, etc) ====");
		dividers.push_back("==== ADD etc family: (ADD/ADC/SUB/SBC etc.) ====");
		dividers.push_back("==== K-series family: (CPI/SBC/ORI/ANDI) ====");
		dividers.push_back("==== K-JMP/CALL family: ====");

		std::vector<test_item> samples;

		samples.push_back(test_item(0x00, ""));
		samples.push_back(test_item(0x9200, "STS R0 -> i"));
		samples.push_back(test_item(0x9010, "LDS i -> r1"));
		samples.push_back(test_item(0x9201, "ST thru Z+"));
		samples.push_back(test_item(0x9209, "ST thru Y+"));
		samples.push_back(test_item(0x9001, "LD thru Z+"));
		samples.push_back(test_item(0x9009, "LD thru Y+"));
		samples.push_back(test_item(0x9202, "ST thru -Z"));
		samples.push_back(test_item(0x920A, "ST thru -Y"));
		samples.push_back(test_item(0x9002, "LD thru -Z"));
		samples.push_back(test_item(0x900A, "LD thru -Y"));
		samples.push_back(test_item(0x9004, "LPM Z"));
		samples.push_back(test_item(0x9006, "ELPM Z"));
		samples.push_back(test_item(0x9005, "LPM Z+"));
		samples.push_back(test_item(0x9007, "ELPM Z+"));
		samples.push_back(test_item(0x900F, "POP"));
		samples.push_back(test_item(0x920F, "PUSH"));
		samples.push_back(test_item(0x9204, "XCH"));
		samples.push_back(test_item(0x9205, "LAS"));
		samples.push_back(test_item(0x9206, "LAC"));
		samples.push_back(test_item(0x9207, "LAT"));
		

		samples.push_back(test_item(0x01, ""));
		samples.push_back(test_item(0x0100, "MOVW"));
		samples.push_back(test_item(0x0200, "MULS"));
		samples.push_back(test_item(0x0300, "MULSU"));
		samples.push_back(test_item(0x0308, "FMUL"));
		samples.push_back(test_item(0x0380, "FMULS"));
		samples.push_back(test_item(0x0388, "FMULSU"));

		samples.push_back(test_item(0x02, ""));
		samples.push_back(test_item(0x9508, "RET"));
		samples.push_back(test_item(0x9518, "RETI"));
		samples.push_back(test_item(0x9588, "SLEEP"));
		samples.push_back(test_item(0x9598, "BREAK"));
		samples.push_back(test_item(0x95A8, "WDR"));
		samples.push_back(test_item(0x95C8, "LPM"));
		samples.push_back(test_item(0x95D8, "ELPM"));
		samples.push_back(test_item(0x95E8, "SPM"));
		samples.push_back(test_item(0x95F8, "SPM_ZP"));

		samples.push_back(test_item(0x03, ""));
		samples.push_back(test_item(0x9500, "COM"));
		samples.push_back(test_item(0x9501, "NEG"));
		samples.push_back(test_item(0x9502, "SWAP"));
		samples.push_back(test_item(0x9503, "INC"));
		samples.push_back(test_item(0x9505, "ASR"));
		samples.push_back(test_item(0x9506, "LSR"));
		samples.push_back(test_item(0x9507, "ROR"));

		samples.push_back(test_item(0x04, ""));
		samples.push_back(test_item(0x0400, "CPC"));
		samples.push_back(test_item(0x1400, "CP"));
		samples.push_back(test_item(0x0800, "SBC"));
		samples.push_back(test_item(0x1800, "SUB"));
		samples.push_back(test_item(0x1C01, "ADC"));
		samples.push_back(test_item(0x0C01, "ADD"));
		samples.push_back(test_item(0x1C00, "ROL"));
		samples.push_back(test_item(0x0C00, "LSL"));
		samples.push_back(test_item(0x1000, "CPSE"));
		samples.push_back(test_item(0x2000, "AND"));
		samples.push_back(test_item(0x2400, "EOR"));
		samples.push_back(test_item(0x2800, "OR"));
		samples.push_back(test_item(0x2C00, "MOV"));

		samples.push_back(test_item(0x05, ""));
		samples.push_back(test_item(0x3000, "CPI"));
		samples.push_back(test_item(0x4000, "SBCI"));
		samples.push_back(test_item(0x5000, "SUBI"));
		samples.push_back(test_item(0x6000, "ORI"));
		samples.push_back(test_item(0x7000, "ANDI"));

		samples.push_back(test_item(0x9509, "INDZ"));
		samples.push_back(test_item(0x9409, "IJMPZ"));
		samples.push_back(test_item(0x9519, "EINDZ"));
		samples.push_back(test_item(0x9419, "EIJMPZ"));
		samples.push_back(test_item(0x940A, "DEC"));
		samples.push_back(test_item(0x940B, "DES"));
		samples.push_back(test_item(0x940E, "CALL ABS22"));
		samples.push_back(test_item(0x940C, "JMP ABS22"));

		//samples.push_back(test_item(, ""));

		for (unsigned int i = 0; i < samples.size(); i++) {
			int first = samples.at(i).first;
			std::string second = samples.at(i).second;

			if (second == "") std::cout << std::endl << dividers.at(first) << std::endl;
			else {
				std::cout << second << std::endl;
				mem.writeWord(0xF00, first);
				emulator.setPC(0xF00);
				emulator.step();
			}
		}
	}

	std::cout << std::endl;
	system("pause");
}