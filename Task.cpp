#include "Task.h"

Task::Task(){ 
	currReqCycles = 0;
	Status = TS_ready;
}

Task::~Task(){
}

void Task::execute() {
	// TODO: 
	//		1. set the start time
	//		2. change the status
}

void EC_Task::execute() {
	Task::execute();
	// TODO:
	//		3. set the suspend/yield event and insert into queue
}
