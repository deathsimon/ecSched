#include<cstdio>
#include<cstring>
#include<list>

#include "ecShed.h"

std::vector<Event*> eventQ;
std::vector<VirCore*> virtualCores;
coreCluster bigCores;
coreCluster littleCores;
double t_now = 0;
double t_sync = t_now;

extern bool EC_schedule_next(PhyCore*, VirCore*);
extern bool EC_schedule_resume(PhyCore*, VirCore*);
extern bool EC_sync();

bool schedule_next(PhyCore* p, VirCore* v){
#ifdef ECBS
	return EC_schedule_next(p, v);
#endif
}
bool schedule_resume(PhyCore* p, VirCore* v){
#ifdef ECBS
	return EC_schedule_resume(p, v);
#endif
}
bool sync(){
#ifdef ECBS
	return EC_sync();
#endif
}

bool setupCoreCluster(coreCluster* cluster, std::string filename){
	
	FILE* fp = fopen(filename.c_str(), "r");
	PhyCore* newPhyCore;
	int amount = 0;
	unsigned int freq;
	double power;
	
	if(!fp){
		fprintf(stderr, "[Error] Cannot open file %s.\n", filename.c_str());
		return false;
	}
	cluster->cores.clear();
	cluster->avFreq.clear();
	cluster->busyPower.clear();
	
	// read amount of cores in the cluster from file
	fscanf(fp, "%d", &amount);
	cluster->amount = amount;
	
	for(int i = 1; i <= amount; i++){
		newPhyCore = new PhyCore(cluster->type, i);		
		cluster->cores.push_back(newPhyCore);
	}

	// read available frequencies and the corresponding busy power
	fscanf(fp, "%d", &amount);
	cluster->amountFreq = amount;
	for(int i = 1; i <= amount; i++){
		fscanf(fp, "%d %lf", &freq, &power);
		cluster->avFreq.push_back(freq);
		cluster->busyPower[freq] = power;
	}
	fclose(fp);

	return true;
}

int main(int argc, char* argv[]){
	bool correct;

	// set up event queue
    eventQ.clear();	
	Event* myEvent = new Event(0, t_interval);
	eventQ.push_back(myEvent);
	// Heapify
	std::make_heap(eventQ.begin(), eventQ.end(), eventOrder());

	// set up big and little core cluster
	bigCores.type = c_big;
	correct = setupCoreCluster(&bigCores, "bigCore.txt");
	littleCores.type = c_little;
	correct &= setupCoreCluster(&littleCores, "littleCore.txt");

	assert(correct);	// check if read core configurations correctly

	// set up virtual cores
	virtualCores.clear();

	VirCore* newVirCore;

	for(int i = 1; i <= N_VIRCORE; i++){	// [TODO] remove N_VIRCORE
#ifdef ECBS
		newVirCore = new ECVirCore(i);
#else
		newVirCore = new VirCore(i);
#endif
		newVirCore->readInput(DIR_NAME);		
		virtualCores.push_back(newVirCore);
	}

	// simulation
	Event* currEvent;	
	while(t_now < T_OBSERVE){

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

	return 0;
}

