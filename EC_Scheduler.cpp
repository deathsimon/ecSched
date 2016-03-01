#include "ecShed.h"

extern std::vector<Event*> eventQ;
extern std::vector<VirCore*> virtualCores;
extern coreCluster bigCores;
extern coreCluster littleCores;
extern double t_now;
extern double t_sync;

extern void genSchedPlan();

bool migrateVCore(VirCore* v, PhyCore* source, PhyCore* dest, bool steal){
	// locate v in source;
	if(source != NULL){
		assert(source->removeFromRunQ(v));
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
VirCore* workloadStealing(PhyCore* p){
	unsigned int pid = p->getPid();
	PhyCore* targetCore;
	VirCore* nextVCore = NULL;			
	coreCluster* targetCluster;

	(p->getType() == c_big)?(targetCluster = &bigCores):(targetCluster = &littleCores);
	
	if(pid > 1){
		// steal from the left
		targetCore = targetCluster->cores[(pid-1)-1];
		nextVCore = targetCore->findRunnable(p);
		if(nextVCore != NULL){
			// migrate the virtual core here
			migrateVCore(nextVCore, nextVCore->currentCore(), p, true);
		}
	}

	if(pid < (targetCluster->cores.size()-1)){
		// steal from the right
		if(nextVCore == NULL 
			|| nextVCore->queryStatus() != vs_ready ){
			targetCore = targetCluster->cores[pid+1];
			nextVCore = targetCore->findRunnable(p);
		}
		if(nextVCore != NULL){
			// migrate the virtual core here
			migrateVCore(nextVCore, nextVCore->currentCore(), p, true);
		}
	}

	return nextVCore;
}
bool execVcore(PhyCore* p, VirCore* v){
	Event* newEvent;
	double exeTime = t_sync - t_now;
	double creditTime = v->queryCredit(p) / C_MAGICNUM;
	double workTime = 0.0;
	
	if(p->getFreq() > 0){
		workTime = v->peekWorkload() / p->getFreq();
		/* big core speed up */
		if(p->getType() == c_big){
			workTime /= v->getSpeedUp();
		}
	}

	if(creditTime < exeTime){
		exeTime = creditTime;
	}
	if(workTime < exeTime){
		exeTime = workTime;
	}

	// create an event for the virtual core and push into event queue
	newEvent = new Event(t_now + exeTime, t_yield, p, v);
	eventQ.push_back(newEvent);
	std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());

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
	/* big core speed up */
	if(p->getType() == c_big){
		workloadProcessed *= v->getSpeedUp();
	}
	double workloadRemain = v->exeWorkload(workloadProcessed);

	double creditConsumed = executionTime*C_MAGICNUM;
	double credit_remain = v->consumeCredit(p, creditConsumed);

	Event* newEvent;
	double waitFor = 0.0;	

	p->stopExe(t_now);

	if(credit_remain > 0.0){		
		if(workloadRemain == 0){
			// waiting for I/O
			waitFor = v->waitIO();
			if(waitFor <= 0){
				// no workload, wait until have workloads
				goto find_next;
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
		std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());
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
				std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());
			}
		}		
	}
find_next:
	if(t_now != t_sync){
		// find next virtual core for execution
		nextVCore = p->findRunnable();		
		
		if(nextVCore == NULL){
			// cannot find runnable from the run queue,
			// try to steal workload from neighbors
			nextVCore = workloadStealing(p);
		}

		if(nextVCore != NULL
			&& nextVCore->queryStatus() == vs_ready){
			// execute the next virtual core
			if(!execVcore(p, nextVCore)){
				return false;
			}			
		}
		// idle the physical core		
	}

	// put the current virtual core to the run-queue of target core
	if(targetCore != NULL){
		migrateVCore(v, p, targetCore, false);
	}
		
	return true;
}

bool EC_schedule_resume(PhyCore* p, VirCore* v){
	PhyCore* currCore = v->currentCore();
	VirCore* tmp;

	// change the virtual core to ready
	if(v->queryStatus() != vs_ready 
		&& !v->changeStatus(vs_ready)){
			return false;		
	}
	if(!currCore->is_running()){
		//tmp = currCore->findRunnable();		

		if(!execVcore(currCore, v)){
		//if(!execVcore(currCore, tmp)){
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
	unsigned int freq;
	double load;
	double bPower;
	for(int i = 1; i <= cluster->amount; i++){
		freq = cluster->cores[i-1]->getFreq();
		load = cluster->cores[i-1]->acquireLoad();
		fprintf(stdout, "%d %.3lf ", freq, load);
		if(freq != 0){
			bPower = cluster->busyPower[freq];
			power_consumption += (load * bPower);
		}
		else{
			if(load != 0){
				fprintf(stderr,"[Error] Core %d has no frequency but with load.\n", i);
			}
		}
	}
	return power_consumption;
}
void checkVcore(){
	PhyCore* source;
	PhyCore* target = NULL;
	VirCore* v;
	for(int i = 1; i <= N_VIRCORE; i++){	// [TODO] replace this
		v = virtualCores[i-1];		
		source = v->currentCore();
		if(source != NULL
			&& v->queryCredit(source) != 0.0){}
		else{
			target = v->coreWCredit();
		}
		if(target != NULL){
			migrateVCore(v, source, target, false);
		}
	}	
}
void resumeCores(coreCluster* cluster, double now){
	Event* newEvent;
	PhyCore* currCore;
	VirCore* vCore;
	for(int i = cluster->amount; i > 0; i--){
		// start from the last physical core
		currCore = cluster->cores[i-1];
		if(currCore->getFreq() != 0){			
			vCore = currCore->findRunnable();
			
			if(vCore == NULL){
				// cannot find runnable virtual core from run queue,
				// steal from the neighbors
				vCore = workloadStealing(currCore);
			}
			if(vCore != NULL
				&& vCore->queryStatus() == vs_ready){
				// create a resume event for the virtual core and push into event queue
				newEvent = new Event(now, t_resume, currCore, vCore);
				eventQ.push_back(newEvent);
				std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());
			}
		}	
	}
}

bool EC_sync(){
	double power_consumption = 0.0;
	double credit_remains = 0.0;
	int hasWork;

	fprintf(stdout, "%.1lf\t", t_now);

	// Stop running cores first
	stopCores(&bigCores, t_now);
	stopCores(&littleCores, t_now);	

	// fetch system information, such as loading, power consumption, ...
	power_consumption += calculatePower(&bigCores);
	power_consumption += calculatePower(&littleCores);
	fprintf(stdout, "\t%lf\t", power_consumption);
	
	// get remaining credits
	for(int i = 1; i <= N_VIRCORE; i++){
		(virtualCores[i-1]->peekWorkload() == 0.0)?
			(hasWork = 0):(hasWork = 1);
		credit_remains += (virtualCores[i-1]->queryCreditReset() * hasWork);
	}
	fprintf(stdout, "%lf\n", credit_remains);

	// generate new scheduling plan
	genSchedPlan();	

	// check virtual cores
	checkVcore();
	
	// resume cores for execution
	resumeCores(&bigCores, t_now);
	resumeCores(&littleCores, t_now);
	
	return true;
}