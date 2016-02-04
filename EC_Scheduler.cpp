#include "Simulator.h"

void EC_schedule_next(PhyCore* p, VirCore* v){

	unsigned int targetCore;

	double credit_remain = v->consumeCredit(p->getPid(), 0.0);
	// [TODO] calculate how much credit consumed!!!!
	
	
	if(credit_remain > 0.0){	

	// waiting for I/O
	}
	else{
		// no credit
		targetCore = v->coreWCredit();
		if(targetCore != 0){
		
		}
		else{
		
		}
	}

	// put the current virtual core back to run-queue

	v->waitIO();
	
	// find next virtual core for execution				

	//nextVCore = 
}