#include <vector>
#include "process.h"

#ifndef CPU_H
#define CPU_H

class CPU{

public:
	CPU();
	CPU(	int t_cs_, int t_RR_, int num_core);

//ACCESSOR	
	Process getCurrentProcess(){return current_process;}
	bool Idle() const {return idle;}
	bool inSwitch();
	bool unLoading() const {return unloading;}
	bool CheckRRSlice(){return rr_counter==t_RR;}
	bool Loading();

//MODIFIER
	void setCore(int core_index, bool state){cores[core_index]=state;}
	void setCounter(long time_ms){counter = time_ms;}
	void setProcess(Process p) {current_process = p;}
	void setIdle(bool state) {idle = state;}

//HELPER
	void CountRRSlice(){rr_counter++;}
	void ResetRRSlice(){rr_counter=0;}
	void Load(Process p);
	void unLoad();
	void burst();
	
private:
	int t_cs;
	int t_RR;
	int rr_counter;
	Process current_process;
	std::vector<bool> cores; 
	long counter;
	long switch_over;
	bool in_switch;
	bool idle;
	bool load;
	bool unloading;
};

#endif