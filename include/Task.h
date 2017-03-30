#pragma once
#include "stdincludes.h"
#include "HMPPlatform.h"

typedef unsigned int ReqCycles;
typedef double EnergyCredit;


class Core;

enum TaskStatus {
	TS_ready = 1,
	TS_running,
	TS_waiting,
	TS_ending
};

class Task {
public:
	Task();
	~Task();

	double getCurrReqCycles();
	double fetchNxtReqCycles();

	void execute();

private:
	WorkloadSeq::iterator nxtReqCycles;
	ReqCycles currReqCycles;
	TaskStatus Status;
	double startTime;
};

class EC_Task : public Task {
public:
	EC_Task();
	void setECredit(Core*, EnergyCredit);
	double fetchECredit(Core*);
	bool hasECredit();
	bool hasECredit(Core*);

	void execute();
private:
	std::map<Core*, EnergyCredit> ECredit;

};

/*
class VirCore {
public:
	VirCore(unsigned int);
	void readInput(std::string);
	unsigned int getID();
	double getSpeedUp();
	void setCore(PhyCore*);
	PhyCore* currentCore();
	unsigned int getExpWorkload();
	double peekWorkload();
	double exeWorkload(double);
	double waitIO();
	vcoreStatus queryStatus();
	bool changeStatus(vcoreStatus);
private:
	unsigned int vid;
	double expectedWorkload;
	vcoreStatus status;
	double speedUp;
	PhyCore* onCore;
	std::deque<inputWorkload*> input_workload_seq;
	std::deque<double> working_seq;
	std::deque<double> waiting_seq;
};

class ECVirCore : public VirCore {
public:
	ECVirCore(unsigned int);
	double queryCreditReset();
	double queryCredit(PhyCore*);
	bool setCredit(PhyCore*, double);
	double consumeCredit(PhyCore*, double);
	PhyCore* coreWCredit();
private:
	std::map<PhyCore*, double> energyCredit;
};

class GTSVirCore : public VirCore {
public:
	GTSVirCore(unsigned int);
	void incAvgLoad(double, double);
	double getAvgLoad();
	void start(double);
	void stop(double);
	double getVirTime();
private:
	double avgLoad;
	double virTime;
	double lastStart;
};
*/