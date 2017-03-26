#include "HMPPlatform.h"

/*
 * Initial the objects in HMPPlatform
 */
HMPPlatform::HMPPlatform() {
	coreClusters.clear();
	eventQ.clear();
	workloadCollection.clear();
}

void HMPPlatform::setup() {
	/* First, Setup the HMP platform according to the configuration file */
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

	/* TODO: read the task arrving time from file */

	/* Add starting and ending event to queue */
	addEvent(event_newTasks, 0.0);
	addEvent(event_end, newConfig.simuLength);
}

void HMPPlatform::loadConfigs(Configs* config) {	
	try	{
		// TODO: load file
	}
	catch (const std::exception& e) {
		std::cout << "[Error] Fail while loading configurations" << std::endl;
		throw e;
	}
	
	config->ClusterPath.clear();
	config->TaskPath.clear();

	// TODO: parse configs and add paths according to configs
	config->ClusterPath.push_back("");
	config->TaskPath.push_back("");
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

	// read the Frequency-Power-Pair of the core
	unsigned int numFPpair = 0;
	fscanf(fp, "%ud", &numFPpair);
	std::vector<FreqPowerPair*> coreFeatrue;
	coreFeatrue.clear();
	while (numFPpair > 0){
		FreqPowerPair* newPair = new FreqPowerPair();
		fscanf(fp, "%ud %lf", &newPair->frequency, &newPair->power);
		coreFeatrue.push_back(newPair);
		numFPpair--;
	}

	for (unsigned int i = 0; i < numCores; i++)	{
		// TODO: create new cores 
		// newCore = new Core();
		// newCore.setFeature();
		newCluster->push_back(newCore);
	}

	coreClusters.push_back(newCluster);

	fclose(fp);	
}

void HMPPlatform::setTasks(std::string path) {

	std::ifstream fp;
	
	try	{
		fp.open(path.c_str(), std::ifstream::in);
	}
	catch (const std::exception&){
		throw "Fail to load config:" + path;
	}

	double speedup = 0.0;
	fp >> speedup;

	unsigned int length = 0;
	fp >> length;

	std::deque<unsigned int> newWorkload;
	newWorkload.clear();

	unsigned int reqCycles = 0;
	for (std::string line; std::getline(fp, line);) {
		reqCycles = std::stoi(line.substr(0, line.find_first_of(" ")));
		newWorkload.push_back(reqCycles);
	}

	workloadCollection.push_back(&newWorkload);

	fp.close();
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

void HMPPlatform::run() {
	/*
	// set up output file
	fout = fopen("..\\result", "w");
	*/

	Event* currEvent;
	
	while (!eventQ.empty()) {
		currEvent = eventQ.front();
		eventQ.pop_front();

		t_now = currEvent->time;

		switch (currEvent->type) {
		case event_newTasks:
			// TODO: add new tasks to task queue 
		case event_schedule:
			// TODO: generate scheduling plan
			break;
		case event_yield:
			break;
		case event_resume:
			break;
		default:
			break;
		}

	};
	/*

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
