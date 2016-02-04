#include<algorithm>
#include<vector>
#include<deque>
#include<map>
#include<String>

#define N_BIGCORE	2
#define N_LITCORE	4
#define N_VIRCORE	10
#define T_PERIOD	1
#define T_INTERRUPT 0.05
#define C_MAGICNUM	100

enum eventType{t_yield = 1, t_interval, t_resume};
enum vcoreStatus{vs_running = 1, vs_waiting, vs_ready, vs_nocredit};
enum coreType{c_big = 1, c_little};

struct inputWorkload{
	unsigned int expWorkload;
	std::deque<unsigned int> working;
	std::deque<unsigned int> waiting;
};

class VirCore;
class PhyCore;

class Event{
public:
	Event();
	Event(double, eventType);
	Event(double, eventType, PhyCore*, VirCore*);
	double getTime();
	eventType getType();
	void getCore(PhyCore**, VirCore**);

	friend bool operator<(Event a, Event b){
		if(a.time != b.time){
			// sort according to time, smaller first
			return a.time < b.time;
		}
		else{
			// same time, sort according to type
			return a.type < b.type;
		}
	}
private:
	double time;
	eventType type;
	PhyCore* p_core;
	VirCore* v_core;
};

class VirCore{
public:
	VirCore(unsigned int);
	unsigned int getID();
	void readInput(std::string);
	unsigned int getExpWorkload();
	double getWorkload();
	double exeWorkload(double);
	double queryCredit(PhyCore*);
	double consumeCredit(PhyCore*, double);
	double waitIO();
	bool toRun();
	vcoreStatus queryStatus();
	PhyCore* coreWCredit();
private:
	unsigned int vid;
	unsigned int expectedWorkload;
	vcoreStatus status;
	std::deque<inputWorkload*> input_workload_seq;
	std::deque<double> working_seq;
	std::deque<double> waiting_seq;
	std::map<PhyCore*, double> energyCredit;
};

class PhyCore{
public:
	PhyCore(coreType, unsigned int);
	unsigned int getPid();
	void setFreq(unsigned int);
	unsigned int getFreq();
	void startExe(double);
	void stopExe(double);
	double getLastStart();
	double acquireLoad();
	void pushRunQ(VirCore*);
	VirCore* popRunQ();
	VirCore* popRunQ(VirCore*);	
	VirCore* findRunnable();
private:
	unsigned int pid;
	coreType type;
	unsigned int freq;
	double lastStart;
	bool running;
	double run_time;
	std::deque<VirCore*> runQueue;
};

