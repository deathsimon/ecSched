#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

#include <list>
#include <algorithm>
#include <vector>
#include <deque>
#include <map>
#include <String>
#include <memory>

#include <assert.h>

/*
#define ECBS
#ifndef ECBS
	#define GTS
#endif

#ifdef GTS
	#define GOV_CONS
	#ifndef GOV_CONS
		#define GOV_PERF
	#endif
#endif

#define N_VIRCORE	20 //1
#define T_OBSERVE	1 //600
#define T_PERIOD	1
#define T_INTERRUPT 0.05
#define C_ROUNDWORK	50
#define C_MAGICNUM	1000
#define MIN_WORK	0.00001
#define MIN_CREDIT	0.0001

#define DIR_NAME	"large\\"//"workload\\"

#define THRESHOLD	1200*4

enum vcoreStatus{vs_running = 1, vs_waiting, vs_ready, vs_nocredit};
enum coreType{c_big = 1, c_little};
enum queuePos{q_head = 1, q_FIFO, q_tail};
*/
//enum eventType { t_yield = 1, t_interval, t_resume };

enum eventType {event_newTasks = 1, event_schedule, event_yield, event_resume, event_end};

class HMPPlatform;
class Core;
class Task;
/*
class VirCore;
class PhyCore;

struct inputWorkload{
	unsigned int expWorkload;
	std::deque<unsigned int> working;
	std::deque<double> waiting;
};
struct coreCluster{
	std::vector<PhyCore*> cores;
	coreType type;
	int amount;
	unsigned int amountFreq;
	std::vector<unsigned int> avFreq;
	std::map<unsigned int, double> busyPower;
};
class Event{
public:
	Event();
	Event(double, eventType);
	Event(double, eventType, PhyCore*, VirCore*);
	double getTime();
	eventType getType();
	void getCore(PhyCore**, VirCore**);
private:
	double time;
	eventType type;
	PhyCore* p_core;
	VirCore* v_core;
};

class PhyCore{
public:
	PhyCore(coreType, unsigned int);
	unsigned int getPid();
	coreType getType();
	void setFreq(unsigned int);
	unsigned int getFreq();
	bool startExe(double);
	void stopExe(double);
	bool is_running();
	double getLastStart();
	void resetLoad();
	double getLoad();
	bool insertToRunQ(VirCore*, queuePos);	
	bool removeFromRunQ(VirCore*);	
	VirCore* findRunnable();
	VirCore* findRunnable(PhyCore*);
private:
	unsigned int pid;
	coreType type;
	unsigned int freq;
	double lastStart;
	bool running;
	double run_time;
	std::deque<VirCore*> runQueue;
};
*/