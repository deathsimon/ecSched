#include<deque>
#include<map>
#include<String>

#define N_BIGCORE	2
#define N_LITCORE	4
#define N_VIRCORE	10

enum eventType{t_yield = 1, t_interval, t_resume};
enum vcoreStatus{vs_running = 1, vs_waiting, vs_nocredit};

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
	Event(unsigned int, eventType);
	Event(unsigned int, eventType, PhyCore*, VirCore*);
	unsigned int getTime();
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
	unsigned int time;
	eventType type;
	PhyCore* p_core;
	VirCore* v_core;
};

Event::Event(){
	time = 0;
	type = t_interval;
	p_core = 0;
	v_core = 0;
}
Event::Event(unsigned int input_time, eventType input_type){
	time = input_time;
	type = input_type;
	p_core = 0;
	v_core = 0;
}
Event::Event(unsigned int input_time, eventType input_type, PhyCore* input_pcore, VirCore* input_vcore){
	time = input_time;
	type = input_type;
	p_core = input_pcore;
	v_core = input_vcore;
}
unsigned int Event::getTime(){
	return time;
}
eventType Event::getType(){
	return type;
}
void Event::getCore(PhyCore** p, VirCore** c){
	(*p) = p_core;
	(*c) = v_core;
}


class VirCore{
public:
	VirCore(unsigned int);
	unsigned int getID();
	void readInput(std::string);
	unsigned int getExpWorkload();
	double getWorkload();
	double queryCredit(unsigned int);
	double consumeCredit(unsigned int, double);
	double waitIO();
	bool toRun();
	vcoreStatus queryStatus();
	unsigned int coreWCredit();
private:
	unsigned int vid;
	unsigned int expectedWorkload;
	vcoreStatus status;
	std::deque<inputWorkload*> input_workload_seq;
	std::deque<double> working_seq;
	std::deque<double> waiting_seq;
	std::map<unsigned int, double> energyCredit;
};
VirCore::VirCore(unsigned int id){
	vid = id;
	expectedWorkload = 0;
	status = vs_nocredit;
	input_workload_seq.clear();
	working_seq.clear();
	waiting_seq.clear();
	energyCredit.clear();
}
unsigned int VirCore::getID(){
	return vid;
}
void VirCore::readInput(std::string){

}
unsigned int VirCore::getExpWorkload(){
	unsigned int expW;
	if(!input_workload_seq.empty()){
		// get the set of workload for the next interval
		inputWorkload* newInput = input_workload_seq.front();
		input_workload_seq.pop_front();
		// add the remaining workloads to the expected workload of the next interval
		expectedWorkload += newInput->expWorkload;
		// add working and waiting sequence at the end of the current one.
		while(!newInput->working.empty()){
			working_seq.push_back(newInput->working.front());
			newInput->working.pop_front();
		};
		while(!newInput->waiting.empty()){
			waiting_seq.push_back(newInput->waiting.front());
			newInput->waiting.pop_front();
		};
		delete newInput;
	}
	// round up the expected workload
	//expW = expectedWorkload;

	return expW;
}
double VirCore::getWorkload(){
	return working_seq.front();
}
double VirCore::queryCredit(unsigned int pid){
	double credit_remain = 0;
	if(energyCredit.find(pid) != energyCredit.end()){
		credit_remain = energyCredit[pid];
	}
	return credit_remain;
}
double VirCore::consumeCredit(unsigned int pid, double c){
	energyCredit[pid] -= c;
	if(energyCredit[pid] == 0.0){
		status = vs_nocredit;
	}
	return energyCredit[pid];
}
double VirCore::waitIO(){
	// return the amount of time waiting for I/O
	double result = -1;
	if(status == vs_waiting){
		fprintf(stderr, "[Error] waitIO(): virtual core %d is already waiting for I/O.\n", vid);		
	}
	else if(status == vs_nocredit){
		fprintf(stderr, "[Warning] waitIO(): virtual core %d has no credit.\n", vid);		
	}
	else{
		status = vs_waiting;
		result = waiting_seq.front();
		waiting_seq.pop_front();
	}
	return result;
}
bool VirCore::toRun(){
	if(status == vs_running){
		fprintf(stderr, "[Error] toRun(): virtual core %d already running.\n", vid);
		return false;
	}
	else if(status == vs_nocredit){
		fprintf(stderr, "[Error] toRun(): virtual core %d has no credit.\n", vid);
		return false;
	}
	else{
		status = vs_running;
	}
	return true;
}
vcoreStatus VirCore::queryStatus(){
	return status;
}

unsigned int VirCore::coreWCredit(){
	// return a physical core this virtual core has credits on
	unsigned int pid = 0;
	for(std::map<unsigned int, double>::iterator it = energyCredit.begin(); it != energyCredit.end(); ++it){
		if(it->second > 0){
			pid = it->first;
			break;
		}
	}
	return pid;
}
class PhyCore{
public:
	PhyCore(unsigned int);
	unsigned int getPid();
	void setFreq(unsigned int);
	unsigned int getFreq();
private:
	unsigned int pid;
	unsigned int freq;
};
PhyCore::PhyCore(unsigned int id){
	pid = id;
}
unsigned int PhyCore::getPid(){
	return pid;
}
void PhyCore::setFreq(unsigned int f){
	freq = f;
}
unsigned int PhyCore::getFreq(){
	return freq;
}