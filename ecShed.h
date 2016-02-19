#include<algorithm>
#include<vector>
#include<deque>
#include<map>
#include<String>

#include <assert.h>

#define N_VIRCORE	1
#define T_OBSERVE	90
#define T_PERIOD	1
#define T_INTERRUPT 0.05
#define C_ROUNDWORK	50
#define C_MAGICNUM	1000
#define MIN_WORK	0.00001

#define THRESHOLD	900*4

enum eventType{t_yield = 1, t_interval, t_resume};
enum vcoreStatus{vs_running = 1, vs_waiting, vs_ready, vs_nocredit};
enum coreType{c_big = 1, c_little};
enum queuePos{q_head = 1, q_FIFO, q_tail};

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

class VirCore{
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
	double queryCreditReset();
	double queryCredit(PhyCore*);
	bool setCredit(PhyCore*, double);
	double consumeCredit(PhyCore*, double);
	double waitIO();	
	vcoreStatus queryStatus();
	bool changeStatus(vcoreStatus);
	PhyCore* coreWCredit();
private:
	unsigned int vid;
	double expectedWorkload;
	vcoreStatus status;
	double speedUp;
	PhyCore* onCore;
	std::deque<inputWorkload*> input_workload_seq;
	std::deque<double> working_seq;
	std::deque<double> waiting_seq;
	std::map<PhyCore*, double> energyCredit;
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
	double acquireLoad();
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

struct eventOrder{
    bool operator()(Event*, Event*);
};
