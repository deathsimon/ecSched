#pragma once

#include "stdincludes.h"
//#include "HMPPlatform.h"
#include "Task.h"

extern HMPPlatform* simulator;

typedef double Time;
typedef std::deque<Task*> runQueue;

enum CoreStatus {
	CS_ready = 1,
	CS_running,
	CS_idling,
	CS_ending
};

enum CoreType {
	CT_little = 1,
	CT_big
};

struct CoreInfo {
	std::string coreName;
	CoreType type;
	CoreStatus status;
	unsigned int OperFrequency;
	CoreInfo() : coreName(""), status(CS_ready), type(CT_little), OperFrequency(0) {};
};

class Core {
public:
	Core();
	Core(std::string, CoreType);
	Core(std::string, CoreType, coreFeature*);

	void getCoreInfo(CoreInfo&);

	void setFrequency(unsigned int);

	void addTask(Task*);

	void execUntilTime(double);
	CoreStatistics getStatistics();
	
	void resumeAtTime(Time);
	void interruptAtTime(Time);

private:
	CoreInfo Info;
	coreFeature* feature;

	Time busy;
	double load;
	runQueue rQueue;

	Task* nextTask();

};