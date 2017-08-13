#include "emulator.h"
#include <iostream>
#include <fstream>
#include <bitset>

Emulator::Emulator() {
	for (int i = 0; i < CONFIGSIZE; i++) {
		this->config.push_back(false);
	}
	for (int i = 0; i < MEMSIZE; i++) {
		this->mem.push_back(nullptr);
	}
}

Emulator::~Emulator() {
}

void Emulator::tick() {
	std::stringstream msg;
	msg << " Tick...";
	log(msg.str(), INFO);
	msg.str(std::string());
		
	cycles++;

	auto end = subscribers.end();
	for (auto plugin = subscribers.begin(); plugin != end; plugin++) {
		(*plugin)->tick();
	}
}

unsigned int Emulator::clockSubscribe(Plugin * client) {
	this->subscribers.push_back(client);
	std::cout << client->getName() << " registered as plugin " << subscribers.size() - 1 << std::endl;
	client->registerHost(this, subscribers.size()-1);
	return subscribers.size()-1;
}

void Emulator::writeByte(int address, byte data, Memtype memory /* 0 */) {
	this->mem[memory]->writeByte(address, data);
}

void Emulator::writeWord(int address, word data, Memtype memory /* 0 */) {
	this->mem[memory]->writeWord(address, data);
}

byte Emulator::readByte(int address, Memtype memory /* 0 */) {
	return this->mem[memory]->readByte(address);
}

word Emulator::readWord(int address, Memtype memory /* 0 */) {
	return this->mem[memory]->readWord(address);
}

void Emulator::setBit(int address, int bitnum, Memtype memory /* 0 */) {
	this->mem[memory]->setBit(address, bitnum);
}
void Emulator::clearBit(int address, int bitnum, Memtype memory /* 0 */) {
	this->mem[memory]->clearBit(address, bitnum);
}

int Emulator::readBit(int address, int bitnum, Memtype memory /* 0 */) {
	return this->mem[memory]->readBit(address, bitnum);
}


void Emulator::handleInterrupts() {
	if (config.at(VERBOSE)) std::cout << "[Host] Handling interrupts." << std::endl;
}

void Emulator::step() {
	if ((mem[0]->readBit(0x5F, 7)) && (cycles % interruptInterval == 0)) handleInterrupts();
	// TODO: Should move to calling generateState/applyState to ensure all operations are sequential
	tick();
}

void Emulator::run() {
	while (this->state == RUN) this->step();
}

void Emulator::setState(Emulator::State newState) {
	this->state = newState;
}

Emulator::State Emulator::getState() {
	return Emulator::RUN;
}

void Emulator::setPC(int pc) {
	this->pc = pc;
}

void Emulator::configure(Config conf, bool state) {
	switch (conf) {
	case VERBOSE:
		this->config.at(VERBOSE) = state;
		break;
	default:
		break;
	}
}

void Emulator::setMemory(Memory * mem, Memtype memory /* PROGRAM */) {
	this->mem[memory] = mem;
}

bool Emulator::getConfig(Config conf) {
	return this->config.at(conf);
}

void Emulator::loadProgram(const char * progFile) {
	std::stringstream msg;
	msg << "[Host] Loading program from '" << progFile << "'..." << std::endl;
	this->log(msg.str());


	int total = 0;

	int addr = 0x00;
	byte x[2];

	std::fstream f;
	f.open(progFile);

	// extract IHEX header

	char type = f.peek();

	if (type == ':') {
		std::cout << "Intel IHEX file detected." << std::endl;

		std::string types[5] = { ".DATA", "EOF", ".SEGMENT_E", ".SEGMENT_START", ".ADDRESS" };

		// : 0e 0000 00 00e204b903b91fef1a95f1f3fbcf [Checksum]

		auto ascii2hex = [](char ascii) {
			if ((ascii >= '0') && (ascii <= '9'))
				ascii = ascii - '0';
			if ((ascii >= 'a') && (ascii <= 'f'))
				ascii = (ascii + 10) - 'a';
			if ((ascii >= 'A') && (ascii <= 'F'))
				ascii = (ascii + 10) - 'A';
			return ascii;
		};

		auto readiHexByte = [&ascii2hex](char * nibbles)->byte {
			nibbles[0] = ascii2hex(nibbles[0]);
			nibbles[1] = ascii2hex(nibbles[1]);
			byte result = (nibbles[0] << 4) | nibbles[1];
			return result;
		};
		
		struct IHEXheader {
			char startCode;
			char count[2];
			char address[4];
			char recordType[2];
		} header;

		int recordType = 0;

		while (recordType != 1) {
			f.read((char*)&header, 9);

			int offset = (readiHexByte(header.address) << 8) | readiHexByte(&header.address[2]);
			recordType = readiHexByte(header.recordType);
			int count = readiHexByte(header.count);

			std::cout << "Processing " << types[recordType]
				<< " (" << std::dec << count << " bytes, offset 0x"
				<< std::hex << offset
				<<")... ";

			addr = offset;

			if (recordType == 0) {
				bool h = false;
				byte high;

				f.read((char *)x, 2);

				for (int i = 0; i < count; i++) {
					if (h) {
						word y = (readiHexByte((char*)x) << 8) | high;

						this->writeWord(addr, y, Emulator::PROGRAM);
						addr += 2;

						f.read((char *)x, 2);
						h = !h;
					}

					else {
						high = readiHexByte((char*)x);
						h = !h;
						f.read((char *)x, 2);
					}
				}

				std::cout << "Checksum OK." << std::endl;
				f.seekg((int)f.tellg() + 2); // skip \n
				total += count;
			}
			else {
				std::cout << "Type is " << types[recordType] << "; skipping " << count << " bytes" << std::endl;
				f.seekg((int)f.tellg() + count);
			}
		}
	}

	std::cout << "[Host] " << std::dec << total << " bytes loaded." << std::endl;

	f.close();
}

unsigned int Emulator::movePC(int offset /* 1 */) {
	this->pc += (offset * instructionSize);
	return this->pc;
}

void Emulator::log(std::string logString, Loglevel priority /* 3 */) {
	if (priority <= this->logLevel) std::cout << logString << std::endl;
}

