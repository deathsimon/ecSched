#include "HMPPlatform.h"

/*
 * Initial the objects in HMPPlatform
 */
HMPPlatform::HMPPlatform() {
	coreClusters.clear();
	eventQ.clear();	
}

void HMPPlatform::setup() {
	/* First, try to setup the HMP platform according to the configuration file */
	try {
		setConfig();
	}
	catch (const std::exception& e) {
		throw e;
	}
	/* Then setup the event queue */
	setEventQ();
}
/* 
 * Setup the HMP platform according to the configuration file 
 */
void HMPPlatform::setConfig() {
	Configs newConfig;
	try {
		loadConfigs(&newConfig);

		for (auto path : newConfig.ClusterPath) {
			setCoreCluster(path);
		}

		for (auto path : newConfig.TaskPath) {
			setTasks(path);
		}
	}
	catch (const std::exception& e) {
		throw e;
	}
}

void HMPPlatform::loadConfigs(Configs* config) {	
	try	{
		// TODO: load file
	}
	catch (const std::exception& e) {
		std::cout << "[Error] Fail while loading configurations" << std::endl;
		throw e;
	}
	// TODO: setup configs
}

void HMPPlatform::setCoreCluster(std::string path) {
		
	FILE* fp = std::fopen(path.c_str(), "r");
	if (!fp) {
		throw "Fail to load config:" + path;
	}

	CoreCluster* newCluster = new CoreCluster();

	unsigned int numCores = 0;
	Core* newCore = nullptr;
	
	// read amount of cores in the cluster from file
	fscanf(fp, "%d", &numCores);

	for (unsigned int i = 0; i < numCores; i++)	{
		//TODO: newCore = new Core();
		newCluster->push_back(newCore);
	}


	/*	
	unsigned int freq;
	double power;
		
	cluster->avFreq.clear();
	cluster->busyPower.clear();		

	// read available frequencies and the corresponding busy power
	fscanf(fp, "%d", &amount);
	cluster->amountFreq = amount;
	for(int i = 1; i <= amount; i++){
		fscanf(fp, "%d %lf", &freq, &power);
		cluster->avFreq.push_back(freq);
		cluster->busyPower[freq] = power;
	}
	*/

	coreClusters.push_back(newCluster);

	fclose(fp);	
}

void HMPPlatform::setTasks(std::string path) {
	/*
	// set up virtual cores
	virtualCores.clear();

	VirCore* newVirCore;

	for(int i = 1; i <= N_VIRCORE; i++){	// [TODO] remove N_VIRCORE
	#ifdef ECBS
	newVirCore = new ECVirCore(i);
	#else
	newVirCore = new GTSVirCore(i);
	#endif
	newVirCore->readInput(DIR_NAME);
	virtualCores.push_back(newVirCore);
	}
	*/
}

void HMPPlatform::setEventQ()
{
	eventQ.clear();
	// TODO: add the first event	

	// TODO: add the ending event
	Event* myEvent = new Event(/*e_endSimu, end_time*/);
	eventQ.push_back(myEvent);
}

void HMPPlatform::run() {
	/*
	// set up output file
	fout = fopen("..\\result", "w");

	// simulation
	Event* currEvent;
	while (!e_endSimu) {
	
	assert(!eventQ.empty());

	// pop next
	std::pop_heap(eventQ.begin(), eventQ.end(), eventOrder());
	currEvent = eventQ.back();
	eventQ.pop_back();

	t_now = currEvent->getTime();

	PhyCore* curr_pCore;
	VirCore* curr_vCore;
	switch(currEvent->getType()){
	case t_yield:
	currEvent->getCore(&curr_pCore, &curr_vCore);
	if(!schedule_next(curr_pCore, curr_vCore)){
	// somethings wrong
	return -1;
	}
	break;
	case t_interval:
	sync();

	t_sync += T_PERIOD;

	myEvent = new Event(t_sync, t_interval);
	eventQ.push_back(myEvent);
	std::push_heap(eventQ.begin(), eventQ.end(), eventOrder());

	break;
	case t_resume:
	// resume virtual core for execution
	currEvent->getCore(&curr_pCore, &curr_vCore);
	if(!schedule_resume(curr_pCore, curr_vCore)){
	// somethings wrong
	return -1;
	}
	break;
	}
	};

	// output results
	fclose(fout);

	*/
}


void HMPPlatform::addEvent(eventType type, double time) {
	Event* newEvent = new Event;
	newEvent->time = time;
	newEvent->type = type;

	auto pos = std::lower_bound(eventQ.begin(), eventQ.end(), time, cmp_time);

	eventQ.insert(pos, newEvent);
}

bool cmp_time(Event* &e, const double &t) {
	return e->time > t;
}