#include <vector>
#include <iostream>
#include "process.h"

Process::Process(){
		proc_id 		= "foo";
		arrival_time 	= 0;
		num_bursts 		= 0;
		num_burst_left 	= 0;
		burst_time 		= 0;
		current_burst 	= 0;
		io_time 		= 0;
		current_io		= 0;
		wait_time		= 0;
		back_in_ready	= 0;
		
		io_ready 		= false;
		finish_io 		= false;
		preempted 		= false;
}

Process::Process(std::string proc_id_, 
		int arrival_time_, 
		int burst_time_,
		int num_bursts_,
		int io_time_){
	proc_id 		= proc_id_;
	arrival_time 	= arrival_time_;
	num_bursts 		= num_bursts_;
	num_burst_left 	= num_bursts_;
	burst_time 		= burst_time_;
	current_burst  	= burst_time_;
	io_time 		= io_time_;
	current_io		= io_time_;
 	wait_time		= 0;
 	back_in_ready 	= arrival_time;

	io_ready 		= false;
	finish_io		= false;
	preempted		= false;
}

//burst process
void Process::CountDownBurst(){
	if(num_burst_left==0){
		std::cerr<<"Process "<<proc_id<<": no burst left"<<std::endl;
		return;
	}
	io_ready = false;
	finish_io = false;
	current_burst--;
	if(current_burst==0){
		num_burst_left--;
		io_ready = true;
	}
}


//Process doing I/O
void Process::CountDownIO(){
	if(current_io<0){
		std::cerr<<"io time negative"<<std::endl;
		return;
	}
	finish_io = false;
	current_io--;
	if(current_io==0){
		finish_io = true;
	}
}


//different sorting rules
bool sortByArrival(const Process &p1, const Process &p2){
	return p1.ArrTime() < p2.ArrTime();
}

bool sortByIOtime(const Process &p1, const Process &p2){
	return p1.CurrentIO()<p2.CurrentIO();
}

bool sortByBurst(const Process &p1, const Process &p2){
	return p1.BurstTime()<p2.BurstTime();
}

