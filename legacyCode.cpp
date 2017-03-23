// From Simulator.cpp

#ifdef ECBS
extern bool EC_schedule_next(PhyCore*, ECVirCore*);
extern bool EC_schedule_resume(PhyCore*, ECVirCore*);
extern bool EC_sync();

bool schedule_next(PhyCore* p, VirCore* v){
	return EC_schedule_next(p, (ECVirCore*)v);
}
bool schedule_resume(PhyCore* p, VirCore* v){
	return EC_schedule_resume(p, (ECVirCore*)v);
}
bool sync(){
	return EC_sync();
}
#endif

#ifdef GTS
extern bool GTS_schedule_next(PhyCore*, GTSVirCore*);
extern bool GTS_schedule_resume(GTSVirCore*);
extern bool GTS_sync();

bool schedule_next(PhyCore* p, VirCore* v){
	return GTS_schedule_next(p, (GTSVirCore*)v);
}
bool schedule_resume(PhyCore* p, VirCore* v){
	return GTS_schedule_resume((GTSVirCore*)v);
}
bool sync(){
	return GTS_sync();
}
#endif