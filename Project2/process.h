#include <vector>
#include <utility>
#include <iostream>

#ifndef PROCESS_H
#define PROCESS_H

class Process{
public:
	//CONSTRUCTOR
	Process();
	Process(char pid_, int memory_, 
			std::vector< std::pair<int, int> > all_bursts_);

	//ACCESSOR
	const char PID() const{return pid;}
	const int GetMemory() const{return memory;}

	//MEMBER FUNCTIONS
	std::pair<int, int> CheckBurstArrival(int time);
	void Postpone(int clock_time, int t_pos);
	void PrintProcessInfo();
	void DoneBurst();
	bool Finished();

//PRIVATE VARIABLES
private:
	char pid;
	int memory;
	std::vector< std::pair<int, int> > all_bursts;

};

//HELPER FUNCTION (NON-MEMBER)
bool sortByPID(const Process &p1, const Process &p2);

#endif