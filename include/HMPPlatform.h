#pragma once
#include<stdincludes.h>

typedef std::vector<Core*> CoreCluster;

struct Configs{	
	std::vector<std::string> path_of_cluster_config;
	std::vector<std::string> path_of_task_config;
};

// TODO: enum eventType { e_check = 1, t_interval, e_endSimu };
struct Event {
	eventType type;
	double	time;
};

bool cmp_time(const Event* &e, const double &t);

class HMPPlatform {
public:
	HMPPlatform();
	void run();	

protected:
	void addEvent(eventType type, double time);

private:
	void loadConfigs(Configs*);	
	void setCoreCluster(std::string);
	void setTasks(std::string);
	void setEventQ();
	
	std::deque<Event*> eventQ;
	std::vector<CoreCluster*> coreClusters;

	time_t t_now;	
};