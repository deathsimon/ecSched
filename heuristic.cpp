#include "Simulator.h"

extern std::vector<Event*> eventQ;
extern std::vector<VirCore*> virtualCores;
extern coreCluster bigCores;
extern coreCluster littleCores;

struct unit{
	VirCore* v;
	double w;
	bool operator<( const unit &target ) const {
		return w < target.w;
    }
};
struct vcoreGroup{
	std::vector<unit*> vCore;
	double remainingWorkloads;
	coreCluster* cluster;
};

bool cmp_speed(const unit* a, const unit* b){
	return (a->v->getSpeedUp() < b->v->getSpeedUp());
}

bool genPlan(vcoreGroup* vcores){
	
	unsigned int targetCore = 1;
	unit* vcoreUnit;
	coreCluster* cluster = vcores->cluster;
	double freq;
	double a_ij;	
	double temp;
	double used;
	double r;
	
	for(int i = 1; i <= cluster->amount; i++){
		cluster->cores[i-1]->setFreq((unsigned int)0);
	}

	// calculate freq and a_{i, j}	
	while(!vcores->vCore.empty()){
		std::pop_heap(vcores->vCore.begin(), vcores->vCore.end());
		vcoreUnit = vcores->vCore.back();
		vcores->vCore.pop_back();
		used = 0.0;
		
		while(vcoreUnit->w > 0){
			// Phase 1: calculate freq
			if(cluster->cores[targetCore-1]->getFreq() == 0){
				freq = 0;
				// condition 1
				(vcoreUnit->w > vcores->vCore.front()->w)?(temp = vcoreUnit->w):(temp = vcores->vCore.front()->w);
				if(freq < temp){
					freq = temp;
				}				
				// condition 2
				temp = vcores->remainingWorkloads / (vcores->cluster->amount - targetCore + 1);
				if(freq < temp){
					freq = temp;
				}
				// condition 3
				temp = vcoreUnit->w / (1 - used);
				if(freq < temp){
					freq = temp;
				}
				// [TODO] adjust to one of the available frequency
				unsigned int cand_freq;
				for(int i = 0; i < (int)cluster->amountFreq ;i++){
					if(freq <= cluster->avFreq[i]){
						continue;
					}
					cand_freq = cluster->avFreq[i-1];
					break;
				};
				cluster->cores[targetCore-1]->setFreq(cand_freq);
				r = freq;
			}

			// Phase 2: distributed credits
			freq = (double)cluster->cores[targetCore-1]->getFreq();

			if(r > vcoreUnit->w){
				a_ij = vcoreUnit->w / freq;
				r -= vcoreUnit->w;
				vcores->remainingWorkloads -= vcoreUnit->w;
				vcoreUnit->w = 0;			
			}
			else{
				vcoreUnit->w -= r;
				used = r / freq;
				vcores->remainingWorkloads -= r;
				r = 0;				
			}
			double credit = a_ij * C_MAGICNUM * T_PERIOD;

			vcoreUnit->v->setCredit(cluster->cores[targetCore-1], credit);

			if(r == 0){
				targetCore++;
			}
		};
	}

	return true;
}

void genSchedPlan(){	
	
	vcoreGroup groupLittle;
	vcoreGroup groupBig;
	
	unit* vcoreUnit;	
	double vcpuWorkload = 0.0;

	groupLittle.vCore.clear();
	groupLittle.remainingWorkloads = 0.0;
	groupLittle.cluster = &littleCores;
	groupBig.vCore.clear();
	groupBig.remainingWorkloads = 0.0;
	groupBig.cluster = &bigCores;

	for(int i = 1; i <= N_VIRCORE; i++){
		vcoreUnit = new unit();
		vcoreUnit->v = virtualCores[i];
		vcpuWorkload = virtualCores[i]->getExpWorkload();
		vcoreUnit->w = vcpuWorkload;
		if(vcpuWorkload < littleCores.avFreq.front()){
			groupLittle.vCore.push_back(vcoreUnit);
			groupLittle.remainingWorkloads += vcpuWorkload;
		}
		else{
			groupBig.vCore.push_back(vcoreUnit);
			groupBig.remainingWorkloads += vcpuWorkload;		
		}
	}
	std::make_heap(groupLittle.vCore.begin(), groupLittle.vCore.end(), cmp_speed);

	// if totalWorkload is larger than the amount little core cluste can provide,
	// divide into two groups
	while(groupLittle.remainingWorkloads > THRESHOLD){
		// pop the one with the most speed up
		std::pop_heap(groupLittle.vCore.begin(), groupLittle.vCore.end(), cmp_speed);
		vcoreUnit = groupLittle.vCore.back();
		groupLittle.vCore.pop_back();
		groupLittle.remainingWorkloads -= vcoreUnit->w;

		// insert into big core group
		groupBig.vCore.push_back(vcoreUnit);		
		groupBig.remainingWorkloads += vcoreUnit->w;
	};
	if(groupBig.vCore.size() > 1){
		std::make_heap(groupBig.vCore.begin(), groupBig.vCore.end());
	}
	if(groupLittle.vCore.size() > 1){
		std::make_heap(groupLittle.vCore.begin(), groupLittle.vCore.end());
	}

	genPlan(&groupLittle);
	genPlan(&groupBig);
}