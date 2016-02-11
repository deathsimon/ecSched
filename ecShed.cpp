#include<sstream>

#include "ecShed.h"

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
	expectedWorkload = 0.0;
	status = vs_nocredit;
	speedUp = 1.0;
	onCore = NULL;
	input_workload_seq.clear();
	working_seq.clear();
	waiting_seq.clear();
	energyCredit.clear();
}
void VirCore::readInput(std::string dirName){	
	std::ostringstream tmpStr;
	tmpStr << vid;
	std::string id = tmpStr.str();
	std::string fileName = dirName + id + ".txt";
	FILE* fp = fopen(fileName.c_str(), "r");

	inputWorkload* newInput;
	unsigned int amount;
	unsigned int pairs;
	unsigned int tempL;
	double tempW;

	fscanf(fp, "%lf", &speedUp);

	fscanf(fp, "%d", &amount);
	while(amount > 0){
		newInput = new inputWorkload();
		newInput->waiting.clear();
		newInput->working.clear();
		// format: [Expected workload] [N pairs] [workload wait]+
		// get Expected workload
		fscanf(fp, "%d", &newInput->expWorkload);
		// get pairs
		fscanf(fp, "%d", &pairs);
		// get workload and wait
		while(pairs > 0){
			fscanf(fp, "%d %lf", &tempL, &tempW);
			newInput->working.push_back(tempL);
			newInput->waiting.push_back(tempW);
			pairs--;		
		};
		input_workload_seq.push_back(newInput);
		amount--;
	};
	fclose(fp);
}
unsigned int VirCore::getID(){
	return vid;
}
double VirCore::getSpeedUp(){
	return speedUp;
}
void VirCore::setCore(PhyCore* p){
	onCore = p;
}
PhyCore* VirCore::currentCore(){
	return onCore;
}
unsigned int VirCore::getExpWorkload(){	
	unsigned int expW = 0;
	if(!input_workload_seq.empty()){
		// get the set of workload for the next interval
		inputWorkload* newInput = input_workload_seq.front();
		input_workload_seq.pop_front();
		// add the remaining workloads to the expected workload of the next interval
		expectedWorkload += (double)newInput->expWorkload;
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
	// round up the workload
	if(expectedWorkload > 0){
		expW = ((int)(expectedWorkload/C_ROUNDWORK)+1) * C_ROUNDWORK;
	}

	return expW;
}
double VirCore::peekWorkload(){
	if(!working_seq.empty())
		return working_seq.front();
	return 0.0;
}
double VirCore::exeWorkload(double w){
	expectedWorkload -= w;
	working_seq[0] -= w;
	if(working_seq.front() <= 0){
		working_seq.pop_front();
		return 0.0;
	}
	return working_seq.front();
}
double VirCore::queryCreditReset(){
	double credit_remain = 0.0;
	for(std::map<PhyCore*, double>::iterator it = energyCredit.begin(); it != energyCredit.end(); ++it){
		credit_remain += it->second;
	}
	energyCredit.clear();
	return credit_remain;
}
double VirCore::queryCredit(PhyCore* p){
	double credit_remain = 0.0;
	if(energyCredit.find(p) != energyCredit.end()){
		credit_remain = energyCredit[p];
	}
	return credit_remain;
}
bool VirCore::setCredit(PhyCore* p, double credit){
	if(energyCredit.find(p) != energyCredit.end()){
		// not cleaned
		return false;
	}
	energyCredit[p] = credit;
	if(status == vs_nocredit){
		status = vs_ready;
	}
	return true;
}
double VirCore::consumeCredit(PhyCore* pid, double c){
	energyCredit[pid] -= c;
	if(energyCredit[pid] == 0.0){
		energyCredit.erase(pid);
		status = vs_nocredit;
		return 0.0;
	}
	return energyCredit[pid];
}
double VirCore::waitIO(){
	// return the amount of time waiting for I/O
	double result = -1;
	if(changeStatus(vs_waiting)
		&& !waiting_seq.empty()){
		result = waiting_seq.front();
		waiting_seq.pop_front();
	}
	return result;
}
vcoreStatus VirCore::queryStatus(){
	return status;
}
bool VirCore::changeStatus(vcoreStatus newStatus){
	bool result = false;
	if(status == newStatus){
		fprintf(stderr, "[Error] changeStatus(): virtual core %d status unchanged.\n", vid);		
	}
	else{
		switch(status){
			case vs_ready:
				if(newStatus != vs_running){
					fprintf(stderr, "[Error] changeStatus(): virtual core %d is not running.\n", vid);
				}
				else{
					result = true;
				}
				break;
			case vs_nocredit:
				if(newStatus != vs_ready){
					fprintf(stderr, "[Error] changeStatus(): virtual core %d has no credit.\n", vid);
				}
				else{
					result = true;
				}
				break;
			case vs_waiting:
				if(newStatus != vs_ready){
					fprintf(stderr, "[Error] changeStatus(): virtual core %d is waiting for I/O.\n", vid);
				}
				else{
					result = true;
				}
				break;
			default:				
				result = true;
				break;
		}		
	}
	if(result){
		status = newStatus;
	}
	return result;
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
bool PhyCore::startExe(double t){
	lastStart = t;
	running = true;
	return runQueue.front()->changeStatus(vs_running);
}
void PhyCore::stopExe(double t){
	if(!running){
		fprintf(stderr, "[Error] stopExe: physical core %d was not running.\n", pid);
		return;
	}
	run_time += (t - lastStart);
	running = false;
}
bool PhyCore::is_running(){
	return running;
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
	v->setCore(this);

	return true;
}
VirCore* PhyCore::peakRunQ(){
	if(runQueue.empty())
		return NULL;
	return runQueue.front();
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
		v->setCore(NULL);
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
bool eventOrder::operator()(Event* a, Event* b){
	if(a->getTime() != b->getTime()){
		return a->getTime() > b->getTime();
	}
	else{
		return a->getType() > b->getType();
	}		
}