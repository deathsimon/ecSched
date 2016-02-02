#include<cstdio>
#include<cstring>
#include<algorithm>
#include<vector>
#include<list>

#include "Simulator.h"

std::vector<Event*> eventQ;
std::list<VirCore*> vcpu_workload;

int main(){
    
	// init()
    eventQ.clear();	
	Event* myEvent = new Event(0, t_interval);
	eventQ.push_back(myEvent);


	// Heapify
	std::make_heap(eventQ.begin(), eventQ.end());

	// simulation
	Event* currEvent;
	Event* nextVCore;
	while(!eventQ.empty()){
		// pop next
		std::pop_heap(eventQ.begin(), eventQ.end());
		currEvent = eventQ.back();
		eventQ.pop_back();
		
		nextVCore = NULL;
		switch(currEvent->getType()){
			case t_interval:
				// generate new scheduling plan

				break;
			case t_yield:
				// find next virtual core for execution				

				//nextVCore = ;
			
				break;
			case t_resume:
				// resume virtual core for execution
				break;
		}
		if(nextVCore != NULL){
			// push new
			eventQ.push_back(nextVCore);
			std::push_heap(eventQ.begin(), eventQ.end());
		}
	};
    
	// output results


	return 0;
}