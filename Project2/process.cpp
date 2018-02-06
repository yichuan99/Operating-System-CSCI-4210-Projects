#include <vector>
#include <utility>
#include "process.h"

//constructor
Process::Process(){
	pid = '-';
	memory = 0;
}

//non-default constructor
Process::Process(char pid_, int memory_, 
				std::vector< std::pair<int, int> > all_bursts_){
	pid = pid_;
	memory = memory_;
	all_bursts = all_bursts_;
}

//check if the process ready for a burst
std::pair<int, int> Process::CheckBurstArrival(int time_){
	for(unsigned int i=0; i<all_bursts.size(); ++i){
		if(all_bursts[i].first==time_){
			return all_bursts[i];
		}
	}
	return std::make_pair(-1,-1);
}

//postpone the process (used for defragmentation)
void Process::Postpone(int clock_time, int t_pos){
	for(unsigned int i=0; i<all_bursts.size(); ++i){
		if(all_bursts[i].first>clock_time)all_bursts[i].first+=t_pos;
	}
}

//helper -- print process info
void Process::PrintProcessInfo(){
	std::cout<<"Process "<<pid<<" ";
	std::cout<<memory;
	for(unsigned int i=0; i<all_bursts.size(); ++i){
		std::cout<<" ";
		std::cout<<all_bursts[i].first;
		std::cout<<"/";
		std::cout<<all_bursts[i].second;
	}
	std::cout<<std::endl;
}

//process finished a burst, remove this burst from the list
void Process::DoneBurst(){
	if(all_bursts.size()>=0)all_bursts.erase(all_bursts.begin());
}

//process finished all bursts
bool Process::Finished(){
	return all_bursts.size()==0;
}

//helper -- sort process by ID
bool sortByPID(const Process &p1, const Process &p2){
	return p1.PID() < p2.PID();
}




