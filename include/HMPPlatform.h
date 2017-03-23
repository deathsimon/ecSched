#pragma once
#include<stdincludes.h>

typedef std::vector<Core*> CoreCluster;

struct Configs{	
	std::vector<std::string> ClusterPath;
	std::vector<std::string> TaskPath;
};

// TODO: enum eventType { e_check = 1, t_interval, e_endSimu };
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
	void setConfig();
	void loadConfigs(Configs*);	
	void setCoreCluster(std::string);
	void setTasks(std::string);

	void setEventQ();
	
	std::deque<Event*> eventQ;
	std::vector<CoreCluster*> coreClusters;

	time_t t_now;	
};