#pragma once

#include<stdincludes.h>

#include "Core.h"
#include "Task.h"

typedef std::vector<Core*> CoreCluster;

struct Configs{	
	unsigned int simuLength;
	std::vector<std::string> ClusterPath;
	std::vector<std::string> TaskPath;
	std::string TaskArrivalPath;
};

struct Event {
	eventType type;
	double	time;
};

bool cmp_time(Event* &e, const double &t);

class HMPPlatform {
public:
	HMPPlatform();
	~HMPPlatform();

	void setup();
	void run();	

	void addEvent(eventType type, double time);

private:
	void loadConfigs(Configs*);	
	void setCoreCluster(std::string);
	CoreType whichCoreType(std::string);	
	void setTasks(std::string);
	void setTaskArrival(std::string);

	void genSchedule();
	void collectStatistics();
	void collectCoreStatistics();
	void collectTaskStatistics();
	void checkNewTasks();
	void updateTasks();
	void genPlan();

	void taskResume();

	void taskYield();
	
	std::deque<Event*> eventQ;
	std::vector<CoreCluster*> coreClusters;
	std::vector<WorkloadSeq*> workloadSet;
	std::vector<Task*> tasksPool;

	double t_now;

	std::ofstream record;
};