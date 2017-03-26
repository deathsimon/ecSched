#pragma once
#include<stdincludes.h>

typedef std::vector<Core*> CoreCluster;
typedef std::deque<unsigned int> WorkloadSeq;

struct Configs{	
	unsigned int simuLength;
	std::vector<std::string> ClusterPath;
	std::vector<std::string> TaskPath;
};
struct FreqPowerPair{
	unsigned int frequency;
	double power;
};

struct Event {
	eventType type;
	double	time;
};

bool cmp_time(Event* &e, const double &t);

class HMPPlatform {
public:
	HMPPlatform();
	void setup();
	void run();	

protected:
	void addEvent(eventType type, double time);

private:
	void loadConfigs(Configs*);	
	void setCoreCluster(std::string);
	void setTasks(std::string);
	
	std::deque<Event*> eventQ;
	std::vector<CoreCluster*> coreClusters;
	std::vector<WorkloadSeq*> workloadCollection;

	time_t t_now;	
};