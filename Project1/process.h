#include <vector>

#ifndef PROCESS_H
#define PROCESS_H

class Process{
public:

//  CONSTRUCTOR
	Process();
	Process(std::string proc_id_, 
			int arrival_time_, 
			int burst_time_,
			int num_bursts_,
			int io_time_);

//  ACCESSOR
	std::string PID() const {return proc_id;}
	int ArrTime() const {return arrival_time;} 

//  Burst functions
	int BurstTime() const {return burst_time;}
	int CurrentBurst() const {return current_burst;}
	int BurstsLeft() const {return num_burst_left;}
	void CountDownBurst();
	void ResetBurst() {current_burst = burst_time;}	

//  IO functions
	int IOTime() const {return io_time;}
	int CurrentIO() const {return current_io;};
	bool IOReady() const {return io_ready;}
	bool FinishIO() const {return finish_io;}
	void CountDownIO();
	void ResetIO() {current_io = io_time;}

//	Preemption functions
	void setPreemption(bool state) {preempted=state;}
	bool Preempted() const {return preempted;}

//  helper function for wait time
	void countWait(){wait_time++;}
	void resetWait(){wait_time=0;}
	int getWait() const {return wait_time;}
	void setBackInReady(int t){back_in_ready=t;}
	int BackInReady() const{return back_in_ready;}

private:
	std::string proc_id;
	int arrival_time;

	int num_bursts;
	int num_burst_left;

	int burst_time;
	int current_burst;

	int io_time;
	int current_io;

	bool io_ready;
	bool finish_io;

	bool preempted;
	int wait_time;

	int back_in_ready;
};

//Different sorting rules
bool sortByArrival(const Process &p1, const Process &p2);
bool sortByIOtime(const Process &p1, const Process &p2);
bool sortByBurst(const Process &p1, const Process &p2);

#endif