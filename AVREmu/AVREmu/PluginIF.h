#pragma once

class EmulatorIF;

class PluginIF {
public:
	virtual void tick();
	virtual void registerHost(EmulatorIF * host);
};