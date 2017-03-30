#include "Task.h"

Task::Task() : Task::Task("") {};
Task::Task(std::string name){
	taskName = name;
	wSeq = nullptr;
	currReqCycles = 0;
	Status = TS_ready;
}

Task::~Task(){
	std::cout << "Task " << taskName << " has finished." << std::endl;
}

void Task::assignWorkload(WorkloadSeq * seq){
	assignWorkload(seq, 1.0);
}
void Task::assignWorkload(WorkloadSeq * seq, double speedUp) {
	assert(seq != NULL);

	wSeq = seq;
	nxtReqCycles = wSeq->begin();
}

/* Output the information of a task in batch.
 */
void Task::getTaskInfo(TaskInfo & tInfo){
	tInfo.taskName = taskName;
	tInfo.status = Status;
	tInfo.currReqCycles = currReqCycles;
}

/* At the begin of every time interval,
 * fetch the required cycles, and accumulate 
 * to the current required cycles
 * (residuals from the last time interval).
 */
double Task::fetchNxtReqCycles(){
	unsigned int cycles = (*nxtReqCycles);
	currReqCycles += (double)cycles;

	nxtReqCycles++;
	if (nxtReqCycles == wSeq->end()){
		/* Reaching the end of the sequence
		 * means that this task has finished.
		 */
		Status = TS_ending;
	}

	return currReqCycles;
}

void Task::execute() {
	// TODO: 
	//	1. set the start time
	//	2. change the status
}

void Task::suspend() {
	// TODO:
	//	1. change the status
	//	2. calculate executed cycles according to freq and time
}

void EC_Task::execute() {
	Task::execute();
	// TODO:
	//	3. set the suspend/yield event and insert into queue
}

void EC_Task::suspend() {
	Task::suspend();
	// TODO:
	//	3. reduce the ECredit
}
