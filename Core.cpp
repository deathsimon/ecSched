#include "Core.h"

Core::Core() : Core::Core("", CT_little, new coreFeature()) {}
Core::Core(std::string name, CoreType type) : Core::Core(name, type, new coreFeature()) {}
Core::Core(std::string name, CoreType type, coreFeature* cFeature) {
	Info.coreName = name;
	Info.type = type;
	Info.OperFrequency = 0;
	Info.status = CS_idling;

	feature = cFeature;

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
void Core::execUntilTime(double t_now){
	/* TODO */
}
CoreStatistics Core::getStatistics(){
	/* At the end of each interval, return the frequency, power, and load information.
	 * Reset the time of busy.
	 */
	double currLoad = load;
	load = 0.0;
	return std::make_tuple(Info.OperFrequency, 0.0,/*power,*/ currLoad);
}
void Core::resumeAtTime(Time){
	/* TODO:
	 *	resume from idle or interrupt.
	 *	Find the next task from queue for execution.
	 */
}
void Core::interruptAtTime(Time){
	/* TODO:
	 *	During an interval, if something happens, 
	 *	interrupt the core.
	 */
}
Task * Core::nextTask(){
	/* TODO:
	 *	return the next task for execution according to the sheduling algorithm.
	 */
	return nullptr;
};
