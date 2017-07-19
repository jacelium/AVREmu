#include "emulator.h"
#include <iostream>
#include <fstream>
#include <bitset>

Emulator::Emulator() {
	for (int i = 0; i < CONFIGSIZE; i++) {
		this->config.push_back(false);
	}
}

Emulator::Emulator(Memory * mem) : Emulator() {
	this->mem = mem;
}

Emulator::~Emulator() {
}

void Emulator::tick() {
	if (this->getConfig(Emulator::VERBOSE)) std::cout << "[Host] Tick..." << std::endl;

	cycles++;

	auto end = subscribers.end();
	for (auto plugin = subscribers.begin(); plugin != end; plugin++) {
		(*plugin)->tick();
	}
}

unsigned int Emulator::clockSubscribe(Plugin * client) {
	this->subscribers.push_back(client);
	client->registerHost(this, subscribers.size()-1);
	return subscribers.size()-1;
}

void Emulator::writeByte(int address, byte data) {
	this->mem->writeByte(address, data);
}

void Emulator::writeWord(int address, word data) {
	this->mem->writeWord(address, data);
}

byte Emulator::readByte(int address) {
	return this->mem->readByte(address);
}

word Emulator::readWord(int address) {
	return this->mem->readWord(address);
}

void Emulator::setBit(int address, int bitnum) {
	this->mem->setBit(address, bitnum);
}
void Emulator::clearBit(int address, int bitnum) {
	this->mem->clearBit(address, bitnum);
}

int Emulator::readBit(int address, int bitnum) {
	return this->mem->readBit(address, bitnum);
}


void Emulator::handleInterrupts() {
	if (config.at(VERBOSE)) std::cout << "[Host] Handling interrupts." << std::endl;
}

void Emulator::step() {
	if ((mem->readBit(0x5F, 7)) && (cycles % interruptInterval == 0)) handleInterrupts();
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

bool Emulator::getConfig(Config conf) {
	return this->config.at(conf);
}

void Emulator::loadProgram(const char * progFile) {
	std::cout << "[Host] Loading program from '" << progFile << "'..." << std::endl;

	int addr = 0;
	byte x[2];

	std::fstream f;
	f.open(progFile);

	while (f) {
		f.read((char *)x, 2);
		word y = (x[0] << 8) | x[1];

		this->writeWord(addr, y);
		addr += 2;
	}

	std::cout << "[Host] " << addr - 2 << " bytes loaded." << std::endl;

	f.close();
}