#include "Simulator.h"

extern std::vector<Event*> eventQ;
extern std::vector<VirCore*> virtualCores;
extern coreCluster bigCores;
extern coreCluster littleCores;
extern double t_now;
extern double t_sync;

extern void genSchedPlan();

bool migrateVCore(VirCore* v, PhyCore* source, PhyCore* dest, bool steal){
	// locate v in source;
	if(!source->removeFromRunQ(v)){
		return false;
	}

	//insertToRunQ
	queuePos pos;
	if(steal){
		pos = q_head;
	}
	else{
		pos = q_FIFO;
	}
	return dest->insertToRunQ(v, pos);	
}
bool execVcore(PhyCore* p, VirCore* v){
	Event* newEvent;
	double exeTime = t_sync - t_now;
	double creditTime = v->queryCredit(p) / C_MAGICNUM;
	double workTime = v->getWorkload() / p->getFreq();

	if(creditTime < exeTime){
		exeTime = creditTime;
	}
	if(workTime < exeTime){
		exeTime = workTime;
	}

	// create an event for the virtual core and push into event queue
	newEvent = new Event(t_now + exeTime, t_yield, p, v);
	eventQ.push_back(newEvent);
	std::push_heap(eventQ.begin(), eventQ.end());

	if(!p->startExe(t_now)){
		return false;
	}
	return true;
}

bool EC_schedule_next(PhyCore* p, VirCore* v){

	PhyCore* targetCore = p;
	VirCore* nextVCore;
	
	double executionTime = t_now - p->getLastStart();

	double workloadProcessed = executionTime*p->getFreq();
	double workloadRemain = v->exeWorkload(workloadProcessed);

	double creditConsumed = executionTime*C_MAGICNUM;
	double credit_remain = v->consumeCredit(p, creditConsumed);

	Event* newEvent;
	double waitFor = 0.0;	

	if(credit_remain > 0.0){		
		if(workloadRemain == 0){
			// waiting for I/O
			waitFor = v->waitIO();
			if(waitFor < 0){
				return false;
			}			
		}
		else{
			targetCore = NULL;
			if(t_now == t_sync){
				// sync point
				waitFor = 0;
			}
			else{
				// being interrupt
				waitFor = T_INTERRUPT;
				// [LATER] work on this later, not going to happen now			
				fprintf(stderr, "[Error] being interrupted, not suppose to happen by now.\n");
				return false;			
			}
		}
		// create a resume event for the virtual core and push into event queue
		newEvent = new Event(t_now + waitFor, t_resume, p, v);
		eventQ.push_back(newEvent);
		std::push_heap(eventQ.begin(), eventQ.end());
	}
	else{
		// no credit		
		if(t_now == t_sync){
			targetCore = NULL;
		}
		else{
			targetCore = v->coreWCredit();
			if(targetCore == NULL){
				// stay in this core
				targetCore = p;
			}
			else{				
				if(workloadRemain == 0){
					waitFor = v->waitIO();
					if(waitFor < 0){
						return false;
					}			
				}
				// create a resume event for the virtual core and push into event queue
				newEvent = new Event(t_now + waitFor, t_resume, targetCore, v);
				eventQ.push_back(newEvent);
				std::push_heap(eventQ.begin(), eventQ.end());
			}
		}		
	}

	if(t_now != t_sync){
		// find next virtual core for execution
		nextVCore = p->findRunnable();
		targetCore = p;
		
		if(nextVCore == NULL){
			// cannot find runnable from the run queue,
			// try to steal workload from neighbors
			unsigned int pid = p->getPid();
			nextVCore = NULL;
			coreCluster* targetCluster;
			(p->getType() == c_big)?(targetCluster = &bigCores):(targetCluster = &littleCores);
			if(pid > 1){
				targetCore = targetCluster->cores[pid-1];
				nextVCore = targetCore->findRunnable(p);
			}
			if(nextVCore == NULL 
				&& pid < (targetCluster->cores.size()-1)){
				targetCore = targetCluster->cores[pid+1];
				nextVCore = targetCore->findRunnable(p);
			}			
		}

		
		if(nextVCore != NULL){
			// migrate the virtual core here
			migrateVCore(nextVCore, targetCore, p, true);

			// execute the next virtual core
			if(!execVcore(p, nextVCore)){
				return false;
			}			
		}
		else{
			// idle the physical core
			p->stopExe(t_now);
		}
	}

	// put the current virtual core to the run-queue of target core
	migrateVCore(v, p, p, false);
		
	return true;
}

bool EC_schedule_resume(PhyCore* p, VirCore* v){		
	if(!v->changeStatus(vs_ready)){
		// ignore: vs_ready -> vs_ready
		if(v->queryStatus() != vs_ready){
			return false;
		}
	}
	if(p->peakRunQ() == v){
		if(!execVcore(p, v)){
			return false;
		}
	}
	return true;
}
void stopCores(coreCluster* cluster, double now){
	for(int i = 1; i <= cluster->amount ; i++){
		if(cluster->cores[i-1]->is_running()){
			cluster->cores[i-1]->stopExe(now);
		}
	}
}
double calculatePower(coreCluster* cluster){
	double power_consumption = 0.0;
	for(int i = 1; i <= cluster->amount; i++){
		power_consumption += cluster->cores[i-1]->acquireLoad() * cluster->cores[i-1]->getFreq();
	}
	return power_consumption;
}
void resumeCores(coreCluster* cluster, double now){
	for(int i = 1; i <= cluster->amount; i++){
		if(cluster->cores[i-1]->peakRunQ() != NULL)
			cluster->cores[i-1]->startExe(t_now);
	}
}

bool EC_sync(){
	double power_consumption = 0.0;
	double credit_remains = 0.0;

	// Stop running cores first
	stopCores(&bigCores, t_now);
	stopCores(&littleCores, t_now);	

	// fetch system information, such as loading, power consumption, ...
	power_consumption += calculatePower(&bigCores);
	power_consumption += calculatePower(&littleCores);
	
	// get remaining credits
	for(int i = 1; i <= N_VIRCORE; i++){
		credit_remains += virtualCores[i]->queryCredit();
	}

	// generate new scheduling plan
	genSchedPlan();	
	
	// resume cores for execution
	resumeCores(&bigCores, t_now);
	resumeCores(&littleCores, t_now);
	
	return true;
}