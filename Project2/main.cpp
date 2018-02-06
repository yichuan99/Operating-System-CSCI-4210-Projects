//File: CSCI-4210 Project 2
//Name: Yichuan Wang, Zishan Huang
//Description: Simulation for memory management
//Last update: 2016/12/12

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <vector>
#include <set>
#include "process.h"
#include "memory.h"

//handle basic process information
struct process_info{
	char pid;
	int burst_left; 
	int memory;
};

//handle reference page information
struct page_info{
	unsigned short page_num;
	int reference_count;
	int time_from_last_access;
};

//helper function
bool SortProcessByPID(	const process_info &p1, const process_info &p2);

//non-contiguous memory management
void ContiguousSimulation(	std::vector <Process> all_process, std::string algorithm);

//contiguous memory management
void NonContiguousSimulation(	std::vector <Process> all_process	);

//virtual memory simulation
void VirtualMemorySimulation(int F, std::vector<int> reference_pages, std::string algorithm);

//page replacement algorithms
int LRU(int F, page_info* phys_memory);
int LFU(int F, page_info* phys_memory);
int OPT(int F, page_info* phys_memory, std::vector<int> reference_pages, int offset);

//helper function
void PhysicalMemoryPrint(int F, page_info* phys_memory);

//convert string to integer
int StoI(const std::string input_);

//main function
int main(int argc, char const *argv[])
{

//input check
	if(argc!=3){
		std::cerr<<"Usage: a.out arg1 arg2\n";
	}

//===============
	//parsing first file
	
	//buffers for parsing
	std::string line;
	char PID;
	int request_memory;
	std::vector<std::string> buffer;

	//open file stream
	std::ifstream input(argv[1]);
	if(!input.good()){
		std::cerr<<"Can't open "<<argv[2];
	}

	std::vector<Process> all_process;
	//parsing loop
	while(std::getline(input, line)){
		buffer.clear();

		//handle comments
		if(line.at(0)=='#')continue;

		std::stringstream ss(line);
		//int num_proc=0;
		if(isdigit(line[0])){
			continue;
		}

		//handle process info
		while(ss){
			std::string a;
			if(!getline(ss, a, ' '))break;
			buffer.push_back(a);
		}

		//convert input string to numbers
		//get process ID
		PID 			= 	buffer[0][0];
		request_memory 	=  	StoI(buffer[1]);
		std::vector< std::pair<int, int> > bursts;
		for(unsigned int i=2; i<buffer.size(); ++i){
			//parse burst information
			int pos = buffer[i].find("/");
			std::string s = buffer[i];
			std::string arrival_str = s.substr(0,pos);
			s.erase(0,pos+1);
			std::string burst_str = s;
			//store burst information
			int arrival = StoI(arrival_str);
			int burst = StoI(burst_str);
			bursts.push_back(std::make_pair(arrival, burst));
		}
		//create process
		Process p(PID, request_memory, bursts);
		//store process
		all_process.push_back(p);
	}
//===============

	//contiguous memory management
	ContiguousSimulation( all_process , "NEXT");
	std::cout<<std::endl;
	ContiguousSimulation( all_process , "BEST");
	std::cout<<std::endl;
	ContiguousSimulation( all_process , "WORST");
	std::cout<<std::endl;

	//non-contiguous memory management
	NonContiguousSimulation( all_process );
	std::cout<<std::endl;

	//parsing second file
	std::vector<int> reference_pages;
	std::ifstream input2(argv[2]);
	if(!input2.good()){
		std::cerr<<"Can't open "<<argv[2];
	}
	std::string num_str;
	while(input2 >> num_str){
		int num = StoI(num_str);
		reference_pages.push_back(num);
	}

	//virtual memory simulation
	VirtualMemorySimulation(3, reference_pages, "OPT");
	std::cout<<std::endl;
	VirtualMemorySimulation(3, reference_pages, "LRU");
	std::cout<<std::endl;
	VirtualMemorySimulation(3, reference_pages, "LFU");

	return 0;
}

void RemoveFinishedProcess(
	std::vector <process_info> &running_process){
	std::vector <process_info>::iterator process_itr=running_process.begin();
	while(process_itr!=running_process.end()){
		if(process_itr->burst_left==0)running_process.erase(process_itr);
		else ++process_itr;
	}
}

//simulation function
void ContiguousSimulation(	std::vector <Process> all_process, std::string algorithm){

	//initialize
	Memory my_memory;
	std::vector <process_info> running_process;
	int clock_time = 0;
	std::vector<process_info> arrived_process;

	if(algorithm == "NEXT")std::cout<<"time 0ms: Simulator started (Contiguous -- Next-Fit)\n";
	if(algorithm == "BEST")std::cout<<"time 0ms: Simulator started (Contiguous -- Best-Fit)\n";
	if(algorithm == "WORST")std::cout<<"time 0ms: Simulator started (Contiguous -- Worst-Fit)\n";

	//loop through processes
	while(1){

		//clear the arrival queue
		arrived_process.clear();

		//receive processes
		for(unsigned int i=0; i<all_process.size(); ++i){

			std::pair<int, int> tmp_proc_burst = 
				all_process[i].CheckBurstArrival(clock_time);

			if(tmp_proc_burst.first!=-1){
				process_info the_process;
				the_process.pid = all_process[i].PID();
				the_process.burst_left = tmp_proc_burst.second;
				the_process.memory = all_process[i].GetMemory();
				arrived_process.push_back(the_process);
			}

		}

		//alphabetical order
		std::sort(arrived_process.begin(), arrived_process.end(), SortProcessByPID);

		//add new process
		for(unsigned int i=0; i<arrived_process.size(); ++i){
			std::cout<<"time "<<clock_time<<"ms: Process "<<arrived_process[i].pid<<" arrived (requires "<<
						arrived_process[i].memory<<" frames)"<<std::endl;

			//have enough space
			if(arrived_process[i].memory<=my_memory.FreeSpace()){
				int index;
				//placement algorithms
				if(algorithm!="NEXT"){
					index = my_memory.BestWorstFit(arrived_process[i].memory, algorithm);
				}else{
					index = my_memory.NextFit(arrived_process[i].memory);
				}
				
				//have a qualified free partition (large enough)
				if(index != -1){
					my_memory.AddProcessCont(arrived_process[i].pid, index, arrived_process[i].memory);
					running_process.push_back(arrived_process[i]);
					std::cout<<"time "<<clock_time<<"ms: Placed process "<<arrived_process[i].pid<<":"<<std::endl;
					my_memory.MemoryPrint();
				}else{//defragmentation
					std::cout<<"time "<<clock_time<<"ms: Cannot place process "
								<<arrived_process[i].pid<<" -- starting defragmentation"<<std::endl;
					std::set<char> moved_processes;

					//run defragmentation
					int t_pos = my_memory.Defragmentation(moved_processes);
					for(unsigned int j=0; j<all_process.size(); ++j){
						all_process[j].Postpone(clock_time, t_pos);
					}
					//time delay
					clock_time+=t_pos;

					std::cout<<"time "<<clock_time<<"ms: Defragmentation complete ("
							<< "moved "<<t_pos<<" frames: ";
					std::set<char>::iterator p_itr = moved_processes.begin();
					moved_processes.erase('.');
					//check moved processes
					while(p_itr!=moved_processes.end()){
						std::cout<<*(p_itr++);
						if(p_itr!=moved_processes.end())std::cout<<", ";
					}
					std::cout<<")"<<std::endl;
					
					//place process
					my_memory.MemoryPrint();
					if(algorithm!="NEXT"){
						index = my_memory.BestWorstFit(arrived_process[i].memory, algorithm);
					}else{
						index = my_memory.NextFit(arrived_process[i].memory);
					}

					my_memory.AddProcessCont(arrived_process[i].pid, index, arrived_process[i].memory);
					running_process.push_back(arrived_process[i]);					
					std::cout<<"time "<<clock_time<<"ms: Placed process "<<arrived_process[i].pid<<":"<<std::endl;
					my_memory.MemoryPrint();
				}
			}else{//not enough space
				//burst skipped (treat as if it was done)
				for(unsigned int j=0; j<all_process.size(); ++j){
					if(all_process[j].PID() == arrived_process[i].pid){
						all_process[j].DoneBurst();
						break;
					}
				}
				//skip process
				std::cout<<"time "<<clock_time<<"ms: Cannot place process "<<arrived_process[i].pid<<" -- skipped!"<<std::endl;
				my_memory.MemoryPrint();
			}
			
		}

		//increment clock time
		++clock_time;	

		//process burst
		for(unsigned int i=0; i<running_process.size(); ++i){
			running_process[i].burst_left--;
		}

		//remove finished process from memory
		std::vector<process_info>::iterator proc_itr = running_process.begin();
		while(proc_itr!=running_process.end()){
			if(proc_itr->burst_left==0){
				for(unsigned int i=0; i<all_process.size(); ++i){
					if(all_process[i].PID() == proc_itr->pid){
						all_process[i].DoneBurst();
					}
				}
				my_memory.RemoveProcess(proc_itr->pid);
				std::cout<<"time "<<clock_time<<"ms: Process "<<proc_itr->pid<<" removed:"<<std::endl;
				running_process.erase(proc_itr);
				my_memory.MemoryPrint();
			}else{
				++proc_itr;
			}
		}

		//check if the simulation is done
		bool all_done = true;
		for(unsigned int i=0; i<all_process.size(); ++i){
			if(!all_process[i].Finished()){
				all_done=false;
				break;
			}
		}		
		if(all_done)break;
	
	}

	//simulation end
	std::cout<<"time "<<clock_time<<"ms: ";
	if(algorithm == "NEXT")std::cout<<"Simulator ended (Contiguous -- Next-Fit)\n";
	if(algorithm == "BEST")std::cout<<"Simulator ended (Contiguous -- Best-Fit)\n";
	if(algorithm == "WORST")std::cout<<"Simulator ended (Contiguous -- Worst-Fit)\n";

}

void NonContiguousSimulation(	std::vector <Process> all_process	){

	//initialize
	Memory my_memory;
	std::vector <process_info> running_process;
	int clock_time = 0;
	std::vector<process_info> arrived_process;

	//simulation start
	std::cout<<"time 0ms: Simulator started (Non-contiguous)\n";
	while(1){
		//clear arrival queue
		arrived_process.clear();

		//check arrival
		for(unsigned int i=0; i<all_process.size(); ++i){

			std::pair<int, int> tmp_proc_burst = 
				all_process[i].CheckBurstArrival(clock_time);

			if(tmp_proc_burst.first!=-1){
				process_info the_process;
				the_process.pid = all_process[i].PID();
				the_process.burst_left = tmp_proc_burst.second;
				the_process.memory = all_process[i].GetMemory();
				arrived_process.push_back(the_process);
			}

		}
		//check arrived process in alphabetical order
		std::sort(arrived_process.begin(), arrived_process.end(), SortProcessByPID);

		//add new process
		for(unsigned int i=0; i<arrived_process.size(); ++i){
			std::cout<<"time "<<clock_time<<"ms: Process "<<arrived_process[i].pid<<" arrived (requires "<<
						arrived_process[i].memory<<" frames)"<<std::endl;
			//have enough space
			if(arrived_process[i].memory<=my_memory.FreeSpace()){
				my_memory.AddProcessNonCont(arrived_process[i].pid, 
						arrived_process[i].memory);
				running_process.push_back(arrived_process[i]);
				std::cout<<"time "<<clock_time<<"ms: Placed process "<<arrived_process[i].pid<<":"<<std::endl;
				my_memory.MemoryPrint();
			}else{//not enough space
				for(unsigned int j=0; j<all_process.size(); ++j){
					if(all_process[j].PID() == arrived_process[i].pid){
						all_process[j].DoneBurst();
						break;
					}
				}
				//skip the process
				std::cout<<"time "<<clock_time<<"ms: Cannot place process "<<arrived_process[i].pid<<" -- skipped!"<<std::endl;
				my_memory.MemoryPrint();
			}
			
		}

		//process burst
		for(unsigned int i=0; i<running_process.size(); ++i){
			running_process[i].burst_left--;
		}

		//increase clock time
		++clock_time;	

		//remove finished process from memory
		std::vector<process_info>::iterator proc_itr = running_process.begin();
		while(proc_itr!=running_process.end()){
			if(proc_itr->burst_left==0){
				for(unsigned int i=0; i<all_process.size(); ++i){
					if(all_process[i].PID() == proc_itr->pid){
						all_process[i].DoneBurst();
					}
				}
				my_memory.RemoveProcess(proc_itr->pid);
				std::cout<<"time "<<clock_time<<"ms: Process "<<proc_itr->pid<<" removed:"<<std::endl;
				running_process.erase(proc_itr);
				my_memory.MemoryPrint();
			}else{
				++proc_itr;
			}
		}	

		//check if simulation is done
		bool all_done = true;
		for(unsigned int i=0; i<all_process.size(); ++i){
			if(!all_process[i].Finished()){
				all_done=false;
				break;
			}
		}		
		if(all_done)break;

	}

	//simulation end
	std::cout<<"time "<<clock_time<<"ms: ";
	std::cout<<"Simulator ended (Non-contiguous)\n";

}

//helper function to sort process by their id
bool SortProcessByPID(	const process_info &p1, 
								const process_info &p2){
	return p1.pid < p2.pid;
}

//virtual memory simulation
void VirtualMemorySimulation(int F, std::vector<int> reference_pages, std::string algorithm){
	//initialize
	int num_page_fault = 0;
	page_info phys_memory[F];
	for(int i=0; i<F; ++i){
		phys_memory[i].reference_count=-1;
	}
	//start
	std::cout<<"Simulating "<<algorithm<<" with fixed frame size of "<<F<<std::endl;
	
	while(reference_pages.size()!=0){
		bool page_fault = true;//	
		bool added = false;		
		for(int i=0; i<F; ++i){//
			if(phys_memory[i].reference_count==-1){//empty physical memory
				
				//avoid repetitive addition
				if(added)break;

				//add process and check page fault
				phys_memory[i].page_num=reference_pages[0];
				phys_memory[i].reference_count=1;
				phys_memory[i].time_from_last_access=0;	
				page_fault=false;
				std::cout<<"referencing page "<<reference_pages[0]<<" ";
				PhysicalMemoryPrint(F, phys_memory);
				std::cout<<" PAGE FAULT (no victim page)"<<std::endl;
				num_page_fault++;
				break;
			}else if(phys_memory[i].page_num==reference_pages[0]){
			//match, no page fault
				phys_memory[i].reference_count++;
				phys_memory[i].time_from_last_access=0;
				page_fault=false;
				added = true;

			}else{
				//increase time from last access
				phys_memory[i].time_from_last_access++;
			}
		}
		if(page_fault){
			//actual page fault
			num_page_fault++;
			std::cout<<"referencing page "<<reference_pages[0]<<" ";
		
			int index;
			//placement algorithm
			if(algorithm=="LRU"){
				index = LRU(F, phys_memory);
				int victim = phys_memory[index].page_num;				
				phys_memory[index].page_num=reference_pages[0];
				phys_memory[index].reference_count=1;
				phys_memory[index].time_from_last_access=0;
				PhysicalMemoryPrint(F, phys_memory);
				std::cout<<" PAGE FAULT (victim page "<<victim<<")"<<std::endl;
	
			}else if(algorithm=="LFU"){
				index = LFU(F, phys_memory);
				int victim = phys_memory[index].page_num;				
				phys_memory[index].page_num=reference_pages[0];
				phys_memory[index].reference_count=1;
				phys_memory[index].time_from_last_access=0;
				PhysicalMemoryPrint(F, phys_memory);
				std::cout<<" PAGE FAULT (victim page "<<victim<<")"<<std::endl;
			}else{//OPT
				index = OPT(F, phys_memory, reference_pages, 0);
				int victim = phys_memory[index].page_num;				
				phys_memory[index].page_num=reference_pages[0];
				phys_memory[index].reference_count=1;
				phys_memory[index].time_from_last_access=0;
				PhysicalMemoryPrint(F, phys_memory);
				std::cout<<" PAGE FAULT (victim page "<<victim<<")"<<std::endl;
			}
	
		}
		//page added
		reference_pages.erase(reference_pages.begin());
	}
	//simulation end
	std::cout<<"End of "<<algorithm<<" simulation (";
	std::cout<< num_page_fault <<" page faults)"<<std::endl;
}

//least recently used
int LRU(int F, page_info* phys_memory){
	int index=0;
	long page=65537;
	int least_recent=0;

	//loop through running pages to find least recently used process
	for(int i=0; i<F; ++i){
		if(phys_memory[i].time_from_last_access > least_recent){
			index = i;
			least_recent = phys_memory[i].time_from_last_access;
			page=phys_memory[i].page_num;
		}else if(phys_memory[i].time_from_last_access == least_recent){
			if(phys_memory[i].page_num<page){
				index=i;
				page=phys_memory[i].page_num;
			}
		}
	}
	return index;
}

//least frequently used
int LFU(int F, page_info* phys_memory){
	int index=0;
	long page = 65537;
	int least_freq=-1;

	//loop through running pages to find least frequently used process
	for(int i=0; i<F; ++i){
		if( (phys_memory[i].reference_count < least_freq) || least_freq==-1){
			index = i;
			least_freq = phys_memory[i].reference_count;
			page=phys_memory[i].page_num;
		}else if(phys_memory[i].reference_count == least_freq){
			if(phys_memory[i].page_num<page){
				index=i;
				page=phys_memory[i].page_num;
			}			
		}
			
	}
	return index;
}

//optimal
int OPT(int F, page_info* phys_memory, std::vector<int> reference_pages, int offset){

	int index=0;
	unsigned int longest_use=0;
	int current_page_num = -1;

	//foward check reference page string to find optimal process
	for(int i=0; i<F; ++i){
		unsigned short page_number = phys_memory[i].page_num;
		unsigned int j;
		for(j=offset; j<reference_pages.size(); ++j){
			if(reference_pages[j]==page_number)break;
		}
		if(j>longest_use){
			index = i; 
			longest_use = j;
			current_page_num = page_number;
		}else if(j==longest_use){
			if(page_number < current_page_num){
				current_page_num = page_number;
				index = i;
			}
		}
	}
	return index;
}

//helper function to print physical memory (virtal memory management)
void PhysicalMemoryPrint(int F, page_info* phys_memory){
	std::cout<<"[mem:";
	for(int i=0; i<F; ++i){
		std::cout<<" ";
		if(phys_memory[i].reference_count==-1){
			std::cout<<".";
		}else{
			std::cout<<phys_memory[i].page_num;
		}
	}
	std::cout<<"]";
}

//helper function to convert string to int
int StoI(const std::string input_){
	std::string input = input_;
	int val;
	std::stringstream ss(input);
	ss >> val;
	return val;
}