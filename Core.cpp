#include "Core.h"

Core::Core() : Core::Core("", CT_little) {}
Core::Core(std::string name, CoreType type){
	Info.coreName = name;
	Info.type = type;
	Info.OperFrequency = 0;
	busy = 0.0;
	load = 0.0;
	rQueue.clear();
}
void Core::getCoreInfo(CoreInfo & cInfo){
	cInfo.coreName = Info.coreName;
	cInfo.OperFrequency = Info.OperFrequency;
	cInfo.status = Info.status;
	cInfo.type = Info.type;
}
void Core::setFrequency(unsigned int reqFreq){
	/* TODO:
	 *	If reqFreq is avaliable, set OperFrequency to reqFreq;
	 *	else set to the avaliable frequency which is larger than reqFreq.
	 */
}
void Core::addTask(Task * task){
	/* TODO:
	 *	Insert task into the runqueue.
	 */
}
void Core::resume(Time){
	/* TODO:
	 *	resume from pause or interrupt.
	 *	Find the next task from queue for execution.
	 */
}
void Core::interrupt(Time){
	/* TODO:
	 *	During an interval, if something happens, 
	 *	interrupt the core.
	 */
}
void Core::pause(Time){
	/* TODO:
	 *	At the end of each interval, pause the execution of Task.
	 *	Update the load of the core and reset the time of busy.
	 */
}
Task * Core::nextTask(){
	/* TODO:
	 *	return the next task for execution according to the sheduling algorithm.
	 */
	return nullptr;
};
