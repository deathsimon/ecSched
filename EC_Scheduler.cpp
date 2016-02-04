#include "Simulator.h"

extern std::vector<Event*> eventQ;
extern double t_now;
extern double t_sync;

bool EC_schedule_next(PhyCore* p, VirCore* v){

	PhyCore* targetCore = p;
	VirCore* nextVCore;
	
	double workloadProcessed = (t_now - p->getLastStart())*p->getFreq();
	double workloadRemain = v->exeWorkload(workloadProcessed);

	double creditConsumed = (t_now - p->getLastStart())*C_MAGICNUM;
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

		double exeTime;
		double creditTime;
		double workTime;
		if(nextVCore != NULL){
			exeTime = t_sync - t_now;
			creditTime = nextVCore->queryCredit(p) / C_MAGICNUM;
			workTime = nextVCore->getWorkload() / p->getFreq();

			if(creditTime < exeTime){
				exeTime = creditTime;
			}

			if(workTime < exeTime){
				exeTime = workTime;
			}

			// create an event for the virtual core and push into event queue
			newEvent = new Event(t_now + exeTime, t_yield, p, nextVCore);
			eventQ.push_back(newEvent);
			std::push_heap(eventQ.begin(), eventQ.end());	
		}
		else{
			// [TODO] steal workload
			// or
			// [TODO] idle the physical core
		}
	}

	// [TODO] put the current virtual core to the run-queue of target core

			
	return true;
}