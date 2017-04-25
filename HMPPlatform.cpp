#include "HMPPlatform.h"

/*
 * Initial the objects in HMPPlatform
 */
HMPPlatform::HMPPlatform() {
	coreClusters.clear();
	eventQ.clear();
	workloadSet.clear();
	tasksPool.clear();

	t_now = 0;
}

HMPPlatform::~HMPPlatform(){
	record.close();
	std::cout << "End of Simulation" << std::endl;
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
	unsigned int amountCluster = 0;
	fp >> amountCluster;

	std::string path = "";
	std::getline(fp, path);
	for (unsigned int i = 0; i < amountCluster; i++) {
		path.clear();
		std::getline(fp, path);
		config->ClusterPath.push_back(path);
	}

	config->TaskPath.clear();
	unsigned int amountTask = 0;
	fp >> amountTask;

	std::getline(fp, path);
	for (unsigned int i = 0; i < amountTask; i++) {
		path.clear();
		std::getline(fp, path);
		config->TaskPath.push_back(path);
	}

	std::getline(fp, path);
	std::getline(fp, config->TaskArrivalPath);

	fp.close();
}

void HMPPlatform::setCoreCluster(std::string path) {
	
	std::ifstream fp;

	try {
		fp.open(path.c_str(), std::ifstream::in);
	}
	catch (const std::exception& e) {
		throw "Fail to load config:" + path;
	}

	CoreCluster* newCluster = new CoreCluster();

	unsigned int numCores = 0;
	Core* newCore = nullptr;
	
	/* get Core type */
	CoreType cType = whichCoreType(path);

	/* read amount of cores in the cluster from file */
	fp >> numCores;

	/* read the Frequency-Power-Pair of the core */
	unsigned int numFPpair = 0;
	fp >> numFPpair;

	coreFeature* cFeatrue = new coreFeature;
	cFeatrue->clear();
	while (numFPpair > 0){
		FreqPowerPair* newPair = new FreqPowerPair();
		fp >> newPair->frequency >> newPair->power;
		cFeatrue->push_back(newPair);
		numFPpair--;
	}

	for (unsigned int i = 0; i < numCores; i++)	{
		newCore = new Core(std::to_string(i), cType, cFeatrue);
		newCluster->push_back(newCore);
	}

	coreClusters.push_back(newCluster);

	fp.close();
}

CoreType HMPPlatform::whichCoreType(std::string path){
	CoreType type;

	if (path.find("big") != std::string::npos) {
		type = CT_big;
	}
	else if (path.find("little") != std::string::npos) {
		type = CT_little;
	}

	return type;
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

	std::deque<unsigned int>* newWorkload = new std::deque<unsigned int>;
	newWorkload->clear();

	unsigned int reqCycles = 0;
	for (std::string line; std::getline(fp, line);) {
		if (!line.empty()) {
			reqCycles = std::stoi(line.substr(0, line.find_first_of(" ")));
			newWorkload->push_back(reqCycles);
		}
	}

	workloadSet.push_back(newWorkload);

	fp.close();
}

void HMPPlatform::setTaskArrival(std::string path){

	std::ifstream fp;

	try {
		fp.open(path.c_str(), std::ifstream::in);
	}
	catch (const std::exception&) {
		throw "Fail to load task arrival path:" + path;
	}

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
	
	try{
		record.open(PATH_OUTPUT, std::ofstream::out);
	}
	catch (const std::exception& e){
		throw e;
	}
	
	Event* currEvent;
	
	while (!eventQ.empty()) {
		currEvent = eventQ.front();
		eventQ.pop_front();

		t_now = currEvent->time;

		switch (currEvent->type) {
		case event_genSchedule:
			HMPPlatform::genSchedule();
			break;
		case event_yield:
			HMPPlatform::taskYield();
			break;
		case event_resume:
			HMPPlatform::taskResume();
			break;
		case event_end:
			HMPPlatform::collectStatistics();
			eventQ.clear();
		default:
			break;
		}
	};
}

void HMPPlatform::genSchedule() {
	/* First, collect the statistics, e.g. load, avg power consumption, ... */
	HMPPlatform::collectStatistics();
	/* Check if there are new arrival tasks */
	HMPPlatform::checkNewTasks();
	/* Update task workloads and generate the scheduling plan for the next interval. */
	HMPPlatform::updateTasks();
	HMPPlatform::genPlan();
	/* Then do what resume does.*/
	HMPPlatform::taskResume();
}

void HMPPlatform::collectStatistics(){
	
	record << std::setprecision(1) << t_now ;

	collectCoreStatistics();
	collectTaskStatistics();

	record << std::endl;
}

void HMPPlatform::collectCoreStatistics(){
	CoreStatistics cStatistic;
	double energyConsumption = 0.0;

	record << '\t';
	for (auto cluster : coreClusters) {
		for (auto core : *cluster) {
			core->execUntilTime(t_now);
			cStatistic = core->getStatistics();
			/* output the frequency and load of the core*/
			record << std::setprecision(0) << std::get<0>(cStatistic) << ' ' \
				<< std::setprecision(3) << std::get<2>(cStatistic) << ' ';
			/* accumulate the energy consumption */
			energyConsumption += std::get<1>(cStatistic)*std::get<2>(cStatistic);
		}
	}
	/* output the overall energy consumption */
	record << std::setprecision(6) << energyConsumption << std::endl;
}

void HMPPlatform::collectTaskStatistics(){
	// TODO
}

void HMPPlatform::checkNewTasks(){
	Task* newTask;
	// TODO
	while (true){
		newTask = new Task();
		tasksPool.push_back(newTask);
	};
}

void HMPPlatform::updateTasks(){
	// TODO
}

void HMPPlatform::genPlan(){
	// TODO
}

void HMPPlatform::taskResume(){
	/* First check all tasks to find the one(s) that can be resumed. */		
	for (auto task : tasksPool) {
		if (task->resumeableAtTime(t_now)) {
			task->resume();
		}
	}
	/* Then check for idle cores.If any, resume the core(s) and find the next executable task(s). */
	for (auto cluster : coreClusters) {
		for (auto core : *cluster) {
			// TODO: If the core is idle, try to resume it and assign with a task.
		}
	}
}

void HMPPlatform::taskYield(){
	/* All (activated) cores check their current running tasks, find the core(s) which yield. */
	for (auto cluster : coreClusters) {
		for (auto core : *cluster) {
			core->execUntilTime(t_now);
		}
	}
	/* Then do what resume does. */
	HMPPlatform::taskResume();
}
