#pragma once

#include "stdincludes.h"
#include "HMPPlatform.h"
#include "Task.h"

typedef double Time;
typedef std::deque<Task*> runQueue;

enum CoreStatus {
	CS_ready = 1,
	CS_running,
	CS_idling,
	CS_ending
};

enum CoreType {
	CT_big = 1,
	CT_little
};

struct CoreInfo {
	std::string coreName;
	CoreType type;
	CoreStatus status;
	CoreInfo() : coreName(""), status(CS_ready) {};
};

class Core {
public:
	Core();
	Core(std::string);

	void getCoreInfo(CoreInfo&);

	void setFrequency(unsigned int);
	unsigned int getFrequency();

	void resume(Time);
	void interrupt(Time);
	void pause(Time);

private:
	CoreInfo Info;
	unsigned int OperFrequency;
	Time busy;
	double load;
	runQueue rQueue;
};