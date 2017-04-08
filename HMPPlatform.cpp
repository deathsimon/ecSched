#include "HMPPlatform.h"

/*
 * Initial the objects in HMPPlatform
 */
HMPPlatform::HMPPlatform() {
	coreClusters.clear();
	eventQ.clear();
	workloadCollection.clear();

	t_now = 0;
}

void HMPPlatform::setup() {
	Configs newConfig;

	try {
		/* First, load the configuration file */
		loadConfigs(&newConfig);
		/* Setup the cores on the HMP platform according to configuration file */
		for (auto path : newConfig.ClusterPath) {
			setCoreCluster(path);
		}
		/* Setup the tasks according to configuration file */
		for (auto path : newConfig.TaskPath) {
			setTasks(path);
		}
		/* Read the task arrving time from file */
		setTaskArrival(newConfig.TaskArrivalPath);
	}
	catch (const std::exception& e) {
		throw e;
	}
	/* Add the ending event into queue */
	addEvent(event_end, newConfig.simuLength);
}

void HMPPlatform::loadConfigs(Configs* config) {
	
	std::ifstream fp;

	try	{
		fp.open(PATH_CONFIG, std::ifstream::in);
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

	fp.close();
}

void HMPPlatform::setCoreCluster(std::string path) {
		
	FILE* fp = std::fopen(path.c_str(), "r");
	if (!fp) {
		throw "Fail to load config:" + path;
	}

	CoreCluster* newCluster = new CoreCluster();

	unsigned int numCores = 0;
	Core* newCore = nullptr;
	
	/* TODO: get Core type */
	// CoreType cType = ;

	// read amount of cores in the cluster from file
	fscanf(fp, "%d", &numCores);

	// read the Frequency-Power-Pair of the core
	unsigned int numFPpair = 0;
	fscanf(fp, "%ud", &numFPpair);
	coreFeature* cFeatrue = new coreFeature;
	cFeatrue->clear();
	while (numFPpair > 0){
		FreqPowerPair* newPair = new FreqPowerPair();
		fscanf(fp, "%ud %lf", &newPair->frequency, &newPair->power);
		cFeatrue->push_back(newPair);
		numFPpair--;
	}

	for (unsigned int i = 0; i < numCores; i++)	{
		// TODO: create new cores 
		// newCore = new Core("", cType, cFeatrue);
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

void HMPPlatform::setTaskArrival(std::string){

	std::ifstream fp;

	/* TODO:
	 *	read the task arrival time,
	 *	create arrival event and add into event queue;
	 */

	fp.close();
}

void HMPPlatform::addEvent(eventType type, double time) {
	Event* newEvent;

	/* Find the (first) event with timestamp no lesser than time*/
	auto pos = std::lower_bound(eventQ.begin(), eventQ.end(), time, cmp_time);

	if ((*pos)->time == time){
		/* If target event has timestamp equal to time,
		 * check for its event type. */
		if ((*pos)->type < type) {
			/* If smaller, replace it with the current type. */
			(*pos)->type = type;
		}
	}
	else {
		/* If target event has timestamp larger than time, 
		 * create a new event and insert to target position. */
		newEvent = new Event;
		newEvent->time = time;
		newEvent->type = type;

		eventQ.insert(pos, newEvent);
	}
}
bool cmp_time(Event* &e, const double &t) {
	return e->time > t;
}

void HMPPlatform::run() {
	/*
	// TODO: set up output file
	fout = fopen("..\\result", "w");
	*/

	Event* currEvent;
	
	while (!eventQ.empty()) {
		currEvent = eventQ.front();
		eventQ.pop_front();

		t_now = currEvent->time;

		switch (currEvent->type) {
		case event_genSchedule:
			// TODO: add new tasks to task queue and generate scheduling plan
			break;
		case event_yield:
			// TODO:
			break;
		case event_resume:
			// TODO:
			break;
		case event_end:
			// TODO: 
			eventQ.clear();
		default:
			break;
		}
	};

	/* output results
	fclose(fout);
	*/
}
