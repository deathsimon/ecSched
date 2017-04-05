#include "Core.h"

Core::Core() : Core::Core("") {}
Core::Core(std::string name){
	Info.coreName = name;
	OperFrequency = 0;
	busy = 0.0;
	load = 0.0;
	rQueue.clear();
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
};
