#include<deque>
#include<map>

enum eventType{t_interval = 1, t_yield, t_resume};

class Event{
public:
	Event();
	Event(unsigned int, eventType);
	Event(unsigned int, eventType, unsigned int, unsigned int);
	unsigned int getTime();
	eventType getType();
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
	unsigned int p_core;
	unsigned int v_core;
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
Event::Event(unsigned int input_time, eventType input_type, unsigned int input_pcore, unsigned int input_vcore){
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


class VirCore{
public:
	VirCore(unsigned int);
	unsigned int getID();
	unsigned int getWorkload();
	void popWorkload();
	unsigned int queryCredit(unsigned int);
private:
	unsigned int vid;
	std::deque<unsigned int> workload_seq;
	std::map<unsigned int, unsigned int> energyCredit;
};
VirCore::VirCore(unsigned int id){
	vid = id;
	workload_seq.clear();
	energyCredit.clear();
}
unsigned int VirCore::getID(){
	return vid;
}
unsigned int VirCore::getWorkload(){
	return workload_seq.front();
}
void VirCore::popWorkload(){
	workload_seq.pop_front();
}
unsigned int VirCore::queryCredit(unsigned int pid){
	unsigned int ans = 0;
	if(energyCredit.find(pid) != energyCredit.end()){
		ans = energyCredit[pid];
	}
	return pid;
}

class PhyCore{
public:
	PhyCore(unsigned int);
	unsigned int getPid();
	void setFreq();
	unsigned int getFreq();
private:
	unsigned int id;
	unsigned int frequency;
};