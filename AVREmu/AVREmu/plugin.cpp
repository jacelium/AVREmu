#include "plugin.h"

bool Plugin::busy() {
	if (this->busycount == 0)
		return false;
	else
		return true;
}

void Plugin::commit(unsigned int cycles) {
	std::cout << "[" << id << "] Committing for " << cycles << std::endl;
	this->busycount = ++cycles;
}

void Plugin::tick() {
	if (!this->busy()) {
		this->generateState();
	}

	if (this->busycount > 0) this->busycount--;
	
	if (!this->busy()) {
		this->applyState();
	}

}

void Plugin::generateState() {
	// generating state
}

void Plugin::applyState() {
	// applying state
}

void Plugin::registerHost(Emulator * host, unsigned int id) {
	this->host = host;
	this->id = id;
}

void Plugin::log(std::string msg) {
	std::cout << "[" << id << "] " << msg << std::endl;
}