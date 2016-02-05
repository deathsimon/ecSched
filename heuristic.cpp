#include "Simulator.h"

extern std::vector<Event*> eventQ;
extern std::vector<VirCore*> virtualCores;
extern std::vector<PhyCore*> bigCores;
extern std::vector<PhyCore*> littleCores;

struct unit{
	VirCore* v;
	double w;
};
bool cmp(const unit* a, const unit* b){
	return (a->w < b->w);
}

void genSchedPlan(){	
	std::vector<unit*> list_of_vcore;
	unit* vcoreUnit;
	double totalWorkload = 0.0;
	double vcpuWorkload = 0.0;

	list_of_vcore.clear();

	for(int i = 1; i <= N_VIRCORE; i++){
		vcoreUnit = new unit();
		vcoreUnit->v = virtualCores[i];
		vcpuWorkload = virtualCores[i]->getExpWorkload();
		vcoreUnit->w = vcpuWorkload; 
		totalWorkload += vcpuWorkload;
		list_of_vcore.push_back(vcoreUnit);
	}
	std::make_heap(list_of_vcore.begin(), list_of_vcore.end(), cmp);

	// divide into two groups

	// calculate f and a_{i, j}
	// [TODO]	
	while(!list_of_vcore.empty()){
		std::pop_heap(list_of_vcore.begin(), list_of_vcore.end(), cmp);		
		vcoreUnit = list_of_vcore.back();
		list_of_vcore.pop_back();
		vcpuWorkload = vcoreUnit->w;
		// Phase 1


		// Phase 2
	
	}
	// distributed credits

}