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
	std::deque<unsigned int> waiting;
};
struct coreCluster{
	std::vector<PhyCore*> cores;
	coreType type;
	int amount;
	unsigned int amountFreq;
	std::vector<unsigned int> avFreq;
};

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
	double getSpeedUp();
	void readInput(std::string);
	unsigned int getExpWorkload();
	double getWorkload();
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
	unsigned int expectedWorkload;
	vcoreStatus status;
	double speedUp;
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
	VirCore* peakRunQ();
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

