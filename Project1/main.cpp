#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <queue>
#include "cpu.h"

//statistic calculation: for each milisecond, increment counting for each process in the ready queue...

//Helper function to print queue information
void print_queue(std::vector<Process> ready_queue){
	std::cout<<"[Q ";
	if(ready_queue.size()==0){
		std::cout<<"empty]"<<std::endl;
		return;
	}
	for(unsigned int i=0; i<ready_queue.size(); ++i){
		std::cout<<ready_queue[i].PID();
		if(i<ready_queue.size()-1)std::cout<<" ";
	}
	std::cout<<"]"<<std::endl;
}

//main simulator for FCFS, SJF and RR
void simulator(	std::ofstream& output, 
				std::vector<Process> store_queue, 
				CPU cpu, char FCFS, char SJF, char RR){ 
	//set up counting variables
	int total_burst_time = 0;
	int total_cs = 0;
	int total_wait = 0;
	int total_turnaround = 0;
	int total_preemption = 0;
	std::vector<Process> io_processes;
	std::vector<Process> ready_queue;
	int global_counter = -1;
	bool new_process_into_IO = false;
	int total_bursts = 0;
	//collecting burst information
	for(unsigned int i=0; i<store_queue.size(); ++i){
		Process p = store_queue[i];
		total_bursts+=p.BurstsLeft();
		total_burst_time+=(p.BurstsLeft()*p.BurstTime());
	}
	
	//simulation starts
	if(FCFS)std::cout<<"time 0ms: Simulator started for FCFS ";
	if(SJF)std::cout<<"time 0ms: Simulator started for SJF ";
	if(RR) std::cout<<"time 0ms: Simulator started for RR ";

	//if everything was empty and CPU was not working, the job should be done
	print_queue(ready_queue);
	while(	!ready_queue.empty()	||	
			!io_processes.empty()	||
			!store_queue.empty()	||
			!cpu.Idle()					){
		//keep track of time in milliseconds
		global_counter++;

		//increment wait time for processes in ready queue
		for(unsigned int i=0; i<ready_queue.size(); ++i){
			if(i==0&&cpu.unLoading())continue;
			ready_queue[i].countWait();
		}

		if(RR)cpu.CountRRSlice(); //if using RR, keep track of time slice
		while(store_queue.size()!=0){
			//process arrives as time goes by
			if(global_counter==store_queue[0].ArrTime()){
				Process p = store_queue[0];
				ready_queue.push_back(p);
				store_queue.erase(store_queue.begin());
				std::cout<<"time "<<global_counter<<"ms: "<<"Process "<<p.PID()<<" arrived ";
				if(SJF)std::sort(ready_queue.begin(), ready_queue.end(), sortByBurst);
				print_queue(ready_queue);
			}else break;
		}

		//update CPU time 		
		cpu.setCounter(global_counter);
		//for each millisecond:
		//if new processes added to IO, sort IO vector first
		if(new_process_into_IO){
			std::sort(io_processes.begin(), io_processes.end(), sortByIOtime);
		}
		//in io vector, check if any processes has done
		//if there are, kick them out and add them back 
		//to ready queue
		//count down io of some other processes
		for(unsigned int i=0; i<io_processes.size(); ++i){
			io_processes[i].CountDownIO();
		}

		//check if any process finished I/O
		while(!io_processes.empty()){
			if(io_processes[0].FinishIO()){
				Process p = io_processes[0];
				std::cout<<"time "<<global_counter<<"ms: Process "<<p.PID()<<" completed I/O ";
				p.setBackInReady(global_counter);
				ready_queue.push_back(p);
				//in case of SJF, sort process acorrding to burst time so that 
				//the shortest job is at first
				if(SJF)std::sort(ready_queue.begin(), ready_queue.end(), sortByBurst);
				print_queue(ready_queue);
				io_processes.erase(io_processes.begin());
			}else break;
		}

		//if CPU is in context switch, skip any CPU operation
		if(cpu.inSwitch())continue;
		//burst current process (countdown)
		if(cpu.Idle()){//if CPU is idle (at beginning, or just finished a process)
		//load process from ready queue
			if(!ready_queue.empty()){
				Process p(ready_queue[0]);
				cpu.Load(p);
				//each loading indicates a context switch
				total_cs++;
				ready_queue.erase(ready_queue.begin());
				//Process loaded, and cpu should be working after 4ms loading
				cpu.setIdle(false);
			}
		}else{
			//check if a burst is done or not

			//if process still running
			if(cpu.getCurrentProcess().CurrentBurst()>0){
				//for RR, check for preeption
				if(!cpu.CheckRRSlice()){
					//if time slice doesn't expire
					Process p = cpu.getCurrentProcess();
					if(p.CurrentBurst()==p.BurstTime()||p.Preempted()){
						std::cout 	<<"time "<<global_counter<<"ms: Process "
									<<p.PID()<<" started using the CPU ";
						print_queue(ready_queue);
						if(RR){
							cpu.ResetRRSlice();
							p.setPreemption(false);
							cpu.setProcess(p);
						}
					}
					//for FCFS and SJF, simply burst process
					cpu.burst();
				}else{
					//if time slice expires
					cpu.ResetRRSlice();
					if(ready_queue.empty()){
						//if no more process waiting, keep going on current process
						std::cout<<"time "<<global_counter<<"ms: Time slice expired; no preemption because ready queue is empty ";
						print_queue(ready_queue);
						cpu.burst();
					}else{
						//if someone is waiting, do preempt current process
						Process p = cpu.getCurrentProcess();
						cpu.setIdle(true);
						cpu.unLoad();
						p.setPreemption(true);
						ready_queue.push_back(p);
						std::cout<<"time "<<global_counter<<"ms: Time slice expired; process "<<p.PID()<<" preempted with "<<p.CurrentBurst()<<"ms to go ";
						print_queue(ready_queue);
						//count on number of preemptions
						total_preemption++;
					}
				}
				new_process_into_IO=false;		
			}else{
				//if the process finishes burst
				Process p = cpu.getCurrentProcess();
				if(p.BurstsLeft()>0){
					//check if it's terminated or not
					std::cout<<"time "<<global_counter<<"ms: Process "<<p.PID()<<" completed a CPU burst; "<<p.BurstsLeft()<<" to go ";
					total_turnaround+=(global_counter-p.BackInReady());
				}
				else{
					std::cout<<"time "<<global_counter<<"ms: Process "<<p.PID()<<" terminated ";
					total_turnaround+=(global_counter-p.BackInReady());
					total_wait+=p.getWait();
				} 
				print_queue(ready_queue);
				//unload this process
				p.ResetBurst();
				p.ResetIO();
				cpu.setIdle(true);
				cpu.unLoad();
				//if have burst left, push the process to I/O
				if(p.BurstsLeft()>0){
					std::cout<<"time "<<global_counter<<"ms: Process "<<p.PID()<<" blocked on I/O until time "<<global_counter+p.IOTime()<<"ms ";
					print_queue(ready_queue);
					io_processes.push_back(p);
					new_process_into_IO = true;
				}
				//std::cout<<"unload process"<<std::endl;
			}
		}
	}
	//ending
	if(FCFS)std::cout<<"time "<<global_counter+4<<"ms: Simulator ended for FCFS";
	if(SJF)std::cout<<"time "<<global_counter+4<<"ms: Simulator ended for SJF";
	if(RR)std::cout<<"time "<<global_counter+4<<"ms: Simulator ended for RR";	

	//statics output stream
	float avg_wait = total_wait/float(total_bursts);
	float avg_turnaround = total_turnaround/float(total_bursts);
	float avg_burst = total_burst_time/float(total_bursts);
	std::cout<<std::endl;
	output<<"Algorithm ";
	if(FCFS)output<<"FCFS"<<std::endl;
	if(SJF)output<<"SJF"<<std::endl;
	if(RR)output<<"RR"<<std::endl; 	
	output<<std::fixed<<std::setprecision(2);
	output<<"-- average CPU burst time: "<<avg_burst<<" ms"<<std::endl;
	output<<"-- average wait time: "<<avg_wait<<" ms"<<std::endl;
	output<<"-- average turnaround time: "<<avg_turnaround<<" ms"<<std::endl;
	output<<"-- total number of context switches: "<<total_cs<<std::endl;
	output<<"-- total number of preemptions: "<<total_preemption<<std::endl;
	//Done for simulation
}

//helper functon: convert string to int
int StoI(const std::string input_){
	std::string input = input_;
	int val;
	std::stringstream ss(input);
	ss >> val;
	return val;
}

int main(int argc, char const *argv[])
{

	if(argc!=3){//error check
		std::cerr<<"Usage: a.exe input.txt output.txt"<<std::endl;
		exit(1);
	}
	std::vector<Process> store_queue;
	//buffers for parsing
	std::string line;
	std::string PID;
	int arrTime, burstTime, numBursts, ioTime;
	std::vector<std::string> buffer;
	std::ifstream input(argv[1]);
	//while loop to read in files
	while(std::getline(input, line)){
		buffer.clear();
		if(line.at(0)=='#')continue;
		std::stringstream ss(line);
		while(ss){
			std::string a;
			if(!getline(ss, a, '|'))break;
			buffer.push_back(a);
		}
		if(buffer.size()!=5){//error check
			std::cerr<<"wrong input!"<<std::endl;
			exit(1);
		}
		PID 		= 	buffer[0];
		arrTime 	=  	StoI(buffer[1]);
		burstTime 	= 	StoI(buffer[2]);
		numBursts	=   StoI(buffer[3]);
		ioTime 		= 	StoI(buffer[4]);
		Process p(PID, arrTime, burstTime, numBursts, ioTime);
		store_queue.push_back(p);
	}

	std::sort(store_queue.begin(), store_queue.end(), sortByArrival);

	//default parameters
	int t_cs = 8; 
	int t_RR = 84; 
	int num_core = 1;
	CPU cpu(t_cs, t_RR, num_core);
	std::ofstream output(argv[2]);

	//simulation for three algorithms
	simulator(output, store_queue, cpu, 1, 0, 0);
	std::cout<<std::endl;
	simulator(output, store_queue, cpu, 0, 1, 0);
	std::cout<<std::endl;
	simulator(output, store_queue, cpu, 0, 0, 1);
	return 0;
}