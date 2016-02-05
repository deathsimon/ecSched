#include<cstdio>
#include<cstring>
#include<list>

#include "Simulator.h"

std::vector<Event*> eventQ;
std::vector<VirCore*> virtualCores;
std::vector<PhyCore*> bigCores;
std::vector<PhyCore*> littleCores;
double t_now = 0;
double t_sync = T_PERIOD;

extern void genSchedPlan();
extern bool EC_schedule_next(PhyCore*, VirCore*);

bool schedule_next(PhyCore* p, VirCore* v){
	return EC_schedule_next(p, v);
}

int main(){

	// set up event queue
    eventQ.clear();	
	Event* myEvent = new Event(0, t_interval);
	eventQ.push_back(myEvent);
	// Heapify
	std::make_heap(eventQ.begin(), eventQ.end());

	// set up big and little core cluster
	bigCores.clear();
	littleCores.clear();
	PhyCore* newPhyCore;
	for(int i = 1; i <= N_BIGCORE; i++){
		newPhyCore = new PhyCore(c_big, i);
		// [TODO] read input here
		bigCores.push_back(newPhyCore);
	}
	for(int i = 1; i <= N_LITCORE; i++){
		newPhyCore = new PhyCore(c_little, i);
		// [TODO] read input here
		littleCores.push_back(newPhyCore);
	}

	// set up virtual cores
	virtualCores.clear();
	VirCore* newVirCore;
	for(int i = 1; i <= N_VIRCORE; i++){
		newVirCore = new VirCore(i);
		// [TODO] read input here
		virtualCores.push_back(newVirCore);
	}

	// simulation
	Event* currEvent;	
	while(!eventQ.empty()){
		// pop next
		std::pop_heap(eventQ.begin(), eventQ.end());
		currEvent = eventQ.back();
		eventQ.pop_back();

		t_now = currEvent->getTime();
				
		PhyCore* curr_pCore;
		VirCore* curr_vCore;
		switch(currEvent->getType()){
			case t_yield:								
				currEvent->getCore(&curr_pCore, &curr_vCore);				
				if(!schedule_next(curr_pCore, curr_vCore)){
					// somethings wrong
					return -1;
				}
				break;
			case t_interval:
				// [TODO] fetch system information, such as loading, power consumption, ...	

				// .stopExe();
				// .acquireLoad();				

				// generate new scheduling plan
				genSchedPlan();				

				// .startExe();

				t_sync += T_PERIOD;

				break;			
			case t_resume:
				// [TODO] resume virtual core for execution
				currEvent->getCore(&curr_pCore, &curr_vCore);

				break;
		}
		/*
		// push new
		eventQ.push_back(nextVCore);
		std::push_heap(eventQ.begin(), eventQ.end());		
		*/
	};
    
	// [TODO] output results


	return 0;
}

/* Class Event */
Event::Event(){
	time = 0.0;
	type = t_interval;
	p_core = 0;
	v_core = 0;
}
Event::Event(double input_time, eventType input_type){
	time = input_time;
	type = input_type;
	p_core = 0;
	v_core = 0;
}
Event::Event(double input_time, eventType input_type, PhyCore* input_pcore, VirCore* input_vcore){
	time = input_time;
	type = input_type;
	p_core = input_pcore;
	v_core = input_vcore;
}
double Event::getTime(){
	return time;
}
eventType Event::getType(){
	return type;
}
void Event::getCore(PhyCore** p, VirCore** c){
	(*p) = p_core;
	(*c) = v_core;
}
/* Class VirCore */
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
double VirCore::exeWorkload(double w){
	working_seq[0] -= w;
	if(working_seq.front() == 0){
		working_seq.pop_front();
		return 0.0;
	}
	return working_seq.front();
}
double VirCore::queryCredit(PhyCore* pid){
	double credit_remain = 0.0;
	if(energyCredit.find(pid) != energyCredit.end()){
		credit_remain = energyCredit[pid];
	}
	return credit_remain;
}
double VirCore::consumeCredit(PhyCore* pid, double c){
	energyCredit[pid] -= c;
	if(energyCredit[pid] == 0.0){
		status = vs_nocredit;
	}
	return energyCredit[pid];
}
double VirCore::waitIO(){
	// return the amount of time waiting for I/O
	double result = -1;
	switch(status){
		case vs_waiting:
			fprintf(stderr, "[Error] waitIO(): virtual core %d is already waiting for I/O.\n", vid);
			break;
		case vs_nocredit:
			fprintf(stderr, "[Warning] waitIO(): virtual core %d has no credit.\n", vid);
			break;
		case vs_ready:
			fprintf(stderr, "[Warning] waitIO(): virtual core %d is not running.\n", vid);
			break;
		default:
			status = vs_waiting;
			result = waiting_seq.front();
			waiting_seq.pop_front();
	}
	return result;
}
bool VirCore::toRun(){
	bool success = false;
	switch(status){
		case vs_running:
			fprintf(stderr, "[Error] toRun(): virtual core %d already running.\n", vid);
			break;
		case vs_nocredit:
			fprintf(stderr, "[Error] toRun(): virtual core %d has no credit.\n", vid);
			break;
		case vs_waiting:
			fprintf(stderr, "[Error] toRun(): virtual core %d is waiting for I/O.\n", vid);
			break;
		default:
			status = vs_running;
			success = true;
	}
	return success;
}
vcoreStatus VirCore::queryStatus(){
	return status;
}
PhyCore* VirCore::coreWCredit(){
	// return a physical core this virtual core has credits on
	PhyCore* pid = NULL;
	for(std::map<PhyCore*, double>::iterator it = energyCredit.begin(); it != energyCredit.end(); ++it){
		if(it->second > 0){
			pid = it->first;
			break;
		}
	}
	return pid;
}
/* Class PhyCore */
PhyCore::PhyCore(coreType t, unsigned int id){
	pid = id;
	type = t;
	freq = 0;
	lastStart = 0.0;
	running = false;
	run_time = 0.0;
	runQueue.clear();
}
unsigned int PhyCore::getPid(){
	return pid;
}
coreType PhyCore::getType(){
	return type;
}
void PhyCore::setFreq(unsigned int f){
	freq = f;
}
unsigned int PhyCore::getFreq(){
	return freq;
}
void PhyCore::startExe(double t){	
	lastStart = t;
	running = true;
}
void PhyCore::stopExe(double t){
	if(!running){
		fprintf(stderr, "[Error] stopExe: physical core %d was not running.\n", pid);
		return;
	}
	run_time += (t - lastStart);
	running = false;
}
double PhyCore::getLastStart(){
	if(running){
		return lastStart;
	}
	return -1;
}
double PhyCore::acquireLoad(){
	double result = run_time/T_PERIOD;
	run_time = 0.0;
	return result;
}
bool PhyCore::insertToRunQ(VirCore* v, queuePos pos){

	if(pos == q_head){
		runQueue.push_front(v);
	}
	else if(pos == q_tail){
		runQueue.push_back(v);
	}
	else{
		std::deque<VirCore*>::iterator it;
		for(it = runQueue.begin(); it != runQueue.begin(); ++it){
			if((*it)->queryStatus() == vs_nocredit){
				break;
			}
		}
		runQueue.insert(it, v);
	}

	return true;
}
VirCore* PhyCore::popRunQ(){
	VirCore* top = runQueue.front();
	runQueue.pop_front();
	return top;
}
bool PhyCore::removeFromRunQ(VirCore* v){
	std::deque<VirCore*>::iterator target = runQueue.end();
	for(std::deque<VirCore*>::iterator it = runQueue.begin(); it != runQueue.end(); ++it){
		if((*it) == v){
			target = it;
		}
	}
	if(target != runQueue.end()){
		runQueue.erase(target);
	}
	else{
		fprintf(stderr, "[Error] popRunQ: cannot find target in physical core %d.\n", pid);
		return false;
	}
	return true;
}
VirCore* PhyCore::findRunnable(){
	VirCore* target = NULL;
	for(std::deque<VirCore*>::iterator it = runQueue.begin(); it != runQueue.end(); ++it){
		if((*it)->queryStatus() == vs_ready){
			target = (*it);
		}	
	}
	return target;
}
VirCore* PhyCore::findRunnable(PhyCore* p){
	VirCore* target = NULL;
	for(std::deque<VirCore*>::iterator it = runQueue.begin(); it != runQueue.end(); ++it){
		if((*it)->queryStatus() == vs_ready
			&& (*it)->queryCredit(p) != 0.0){
			target = (*it);
		}	
	}
	return target;
}