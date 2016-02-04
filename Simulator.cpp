#include<cstdio>
#include<cstring>
#include<algorithm>
#include<vector>
#include<list>

#include "Simulator.h"

std::vector<Event*> eventQ;
std::list<VirCore*> vcpu_workload;
std::vector<PhyCore*> bigCores;
std::vector<PhyCore*> littleCores;

extern void genSchedPlan();
extern void EC_schedule_next(PhyCore*, VirCore*);

void schedule_next(PhyCore* p, VirCore* v){
	EC_schedule_next(p, v);
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
		newPhyCore = new PhyCore(i);
		bigCores.push_back(newPhyCore);
	}
	for(int i = 1; i <= N_LITCORE; i++){
		newPhyCore = new PhyCore(i);
		littleCores.push_back(newPhyCore);
	}

	// set up virtual cores
	vcpu_workload.clear();
	VirCore* newVirCore;
	for(int i = 1; i <= N_VIRCORE; i++){
		newVirCore = new VirCore(i);
	}


	// simulation
	Event* currEvent;
	// Event* nextEvent;
	while(!eventQ.empty()){
		// pop next
		std::pop_heap(eventQ.begin(), eventQ.end());
		currEvent = eventQ.back();
		eventQ.pop_back();
		
		//nextVCore = NULL;
		PhyCore* curr_pCore;
		VirCore* curr_vCore;
		switch(currEvent->getType()){
			case t_yield:								
				currEvent->getCore(&curr_pCore, &curr_vCore);
				schedule_next(curr_pCore, curr_vCore);
				break;
			case t_interval:
				// fetch system information, such as loading, power consumption, ...

				// generate new scheduling plan
				genSchedPlan();

				break;			
			case t_resume:
				// resume virtual core for execution
				currEvent->getCore(&curr_pCore, &curr_vCore);

				break;
		}
		/*
		// push new
		eventQ.push_back(nextVCore);
		std::push_heap(eventQ.begin(), eventQ.end());		
		*/
	};
    
	// output results


	return 0;
}