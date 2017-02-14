#include "ecShed.h"

#define	T_SLICE_MS	50
#define L_HIGH		0.85
#define L_LOW		0.3
#define	W_RATIO		0

extern std::vector<Event*> eventQ;
extern std::vector<VirCore*> virtualCores;
extern coreCluster bigCores;
extern coreCluster littleCores;
extern double t_now;
extern double t_sync;

extern FILE* fout;

extern void stopCores(coreCluster* cluster, double now);
extern double calculatePower(coreCluster* cluster);
extern void resetLoad(coreCluster* cluster);

std::vector<GTSVirCore*> bigRTtree;
std::vector<GTSVirCore*> littleRTtree;

struct rtOrder{
	bool operator()(GTSVirCore* a, GTSVirCore* b){
		return a->getVirTime() > b->getVirTime();
	}
};

std::vector<GTSVirCore*>* getRTtree(coreType t){
	std::vector<GTSVirCore*>* rtTree;
	(t == c_big)?
		(rtTree = &bigRTtree):(rtTree = &littleRTtree);
	return rtTree;
}
void RTinsert(std::vector<GTSVirCore*>* rt, GTSVirCore* v){
	rt->push_back(v);
	std::push_heap(rt->begin(), rt->end(), rtOrder());
}
GTSVirCore* RTget(std::vector<GTSVirCore*>* rt){
	GTSVirCore* v =  NULL;
	if(!rt->empty()){
		// pop next
		std::pop_heap(rt->begin(), rt->end(), rtOrder());
		v = (GTSVirCore*)rt->back();
		rt->pop_back();		
	}
	return v;
}

bool execVcore(PhyCore* p, GTSVirCore* v){
	Event* newEvent;
	double exeTime;		
	double workTime = 0.0;

	((t_sync - t_now) > (double)T_SLICE_MS/1000)?		// ms 
		(exeTime = (double)T_SLICE_MS/1000):(exeTime = t_sync - t_now);
	
	if(p->getFreq() > 0){
		workTime = v->peekWorkload() / p->getFreq();
		/* big core speed up */
		if(p->getType() == c_big){
			workTime /= v->getSpeedUp();
		}
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
	else{
		assert(v->changeStatus(vs_running));
	}
	return true;
}

bool GTS_schedule_next(PhyCore* p, GTSVirCore* v){

	PhyCore* targetCore = p;
	GTSVirCore* nextVCore;

	std::vector<GTSVirCore*>* rtTree = getRTtree(p->getType());
	
	double executionTime = t_now - p->getLastStart();

	double workloadProcessed = executionTime*p->getFreq();
	/* big core speed up */
	if(p->getType() == c_big){
		workloadProcessed *= v->getSpeedUp();
	}
	double workloadRemain = v->exeWorkload(workloadProcessed);

	Event* newEvent;
	double waitFor = 0.0;	

	p->stopExe(t_now);

	if(workloadRemain == 0){
		// waiting for I/O
		assert(v->changeStatus(vs_waiting));	// vs_running -> vs_waiting
		waitFor = v->waitIO();
		if(waitFor <= 0){
			// no workload, wait until next sync point
			waitFor = t_sync - t_now;
		}			
	}
	else{
		v->changeStatus(vs_ready);
		if(t_now == t_sync){
			// sync point
			waitFor = 0;
		}
		else{
			// T_SLICE_MS expire						
			v->stop(t_now);
			// insert back to RT-tree			
			RTinsert(rtTree, v);

			goto find_next;
		}
	}
	// create a resume event for the virtual core and push into event queue
	newEvent = new Event(t_now + waitFor, t_resume, p, v);
	eventQ.push_back(newEvent);
	std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());
	
find_next:
	if(t_now != t_sync){
		// find next virtual core for execution
		nextVCore = RTget(rtTree);
				
		if(nextVCore != NULL){

			assert(nextVCore->queryStatus() == vs_ready);

			// execute the next virtual core
			nextVCore->setCore(p);
			if(!execVcore(p, nextVCore)){
				return false;
			}
		}
		// idle the physical core		
	}

	return true;
}

void resumeCores(coreCluster* cluster){	
	PhyCore* currCore;
	GTSVirCore* vCore = NULL;
	std::vector<GTSVirCore*>* tree = getRTtree(cluster->type);
	
	for(int i = 0; i < cluster->amount; i++){
		currCore = cluster->cores[i];

		if(!currCore->is_running()
			&& !tree->empty()){
				vCore = RTget(tree);			
		}	

		if(vCore != NULL
			&& vCore->queryStatus() == vs_ready){
				vCore->setCore(currCore);
				execVcore(currCore, vCore);
		}

		if(tree->empty()){
			break;
		}
	}
}

bool GTS_schedule_resume(GTSVirCore* v){

	std::vector<GTSVirCore*>* tree = &bigRTtree;
	coreCluster* cluster = &bigCores;

	if(v->peekWorkload() == 0.0){
		return true;
	}
		
	// change the virtual core to ready
	if(v->queryStatus() != vs_ready){
		assert(v->changeStatus(vs_ready));
	}

	// insert back to tree accordin to avg load
	// [LATER] fix threshold
	//if(v->getAvgLoad() < L_HIGH*littleCores.avFreq[0]){
	if(v->getAvgLoad() < littleCores.avFreq[0]){
		tree = &littleRTtree;
		cluster = &littleCores;
	}		
	RTinsert(tree, v);

	resumeCores(cluster);
	
	return true;
}
void GTS_checkVcore(){
	PhyCore* source;
	PhyCore* target = NULL;
	GTSVirCore* v;
	double vcpuWorkload;

	for(int i = 1; i <= N_VIRCORE; i++){	// [TODO] replace this
		v = (GTSVirCore*)virtualCores[i-1];
		vcpuWorkload = v->getExpWorkload();

		// update the load of each virtual core
		v->incAvgLoad(W_RATIO, vcpuWorkload);
	}	
}
void dvfsCores(coreCluster* cluster){
	PhyCore* currCore;
	double load;
	unsigned int currFreq;

	for(int i = 0; i < cluster->amount; i++){
		currCore = cluster->cores[i];
		currFreq = currCore->getFreq();
		load = currCore->getLoad();

#ifdef GOV_CONS
		if(load >= L_HIGH){
			for(int j = 1; j < (int)cluster->amountFreq ;j++){
				if(currFreq == cluster->avFreq[j]){
					currCore->setFreq(cluster->avFreq[j-1]);
					break;
				}
			}		
		}
		else if(load < L_LOW){
			unsigned int m = (int)cluster->amountFreq;
			currCore->setFreq(cluster->avFreq[m-1]);
		}
#else
		currCore->setFreq(cluster->avFreq[0]);
#endif

		currCore->resetLoad();
	}
}
void initFreq(coreCluster* cluster){
	unsigned int initFreq = cluster->avFreq[0];
	for(int i = 1; i <= cluster->amount; i++){
		cluster->cores[i-1]->setFreq(initFreq);
	}
}

bool GTS_sync(){
	double power_consumption = 0.0;	
	double workload_remains = 0.0;	
	unsigned int hasWork = 0;
	VirCore* v;

	fprintf(stdout, "%.1lf\t", t_now);
	fprintf(fout, "%.1lf\t", t_now);

	// Stop running cores first
	stopCores(&bigCores, t_now);
	stopCores(&littleCores, t_now);	

	// fetch system information, such as loading, power consumption, ...
	power_consumption += calculatePower(&bigCores);
	power_consumption += calculatePower(&littleCores);
	fprintf(stdout, "\t%lf\t", power_consumption);
	fprintf(fout, "\t%lf\t", power_consumption);
	
	// get remaining works
	for(int i = 1; i <= N_VIRCORE; i++){
		if(virtualCores[i-1]->peekWorkload() > 0.0){
			hasWork++;
			workload_remains += virtualCores[i-1]->peekWorkload();
		}
	}
	fprintf(stdout, "%d\t%lf\n", hasWork, workload_remains);
	fprintf(fout, "%d\t%lf\n", hasWork, workload_remains);

	// check virtual cores
	GTS_checkVcore();

	// set core frequency
	if(t_now != 0){
		// adjust core frequency
		dvfsCores(&bigCores);
		dvfsCores(&littleCores);
	}
	else{
		// initial the frequency of cores
		initFreq(&bigCores);
		initFreq(&littleCores);
		
		bigRTtree.clear();
		littleRTtree.clear();

		Event* newEvent;
		PhyCore* p = NULL;
		for(int i = 1; i <= N_VIRCORE; i++){	// [TODO] replace this
			// create resume events for all virtual cores
			v = (GTSVirCore*)virtualCores[i-1];
			newEvent = new Event(t_now, t_resume, p, v);
			eventQ.push_back(newEvent);
			std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());
		}		
	}

	// reset load
	resetLoad(&bigCores);
	resetLoad(&littleCores);

	// resume cores for execution
	//resumeCores(&bigCores, t_now);
	//resumeCores(&littleCores, t_now);
	
	return true;
}