#include <vector>
#include <utility>
#include <map>
#include <cassert>
#include <set>

#ifndef MEMORY_H
#define MEMORY_H

class Memory{
public:
	//CONSTRUCTOR
	Memory();
	Memory(int size);

	//MEMBER FUNCTIONS
	void MemoryPrint(int row_length=32);
	int FreeSpace();
	int MoveFrame(int frame_index);
	int RemoveProcess(char pid);
	int AddProcessCont(char pid, int offset, int length);
	void AddProcessNonCont(char pid, int length);
	int Defragmentation(std::set<char> & moved_processes);
	bool Empty(){return FreeSpace()==mem_size;}
	//contiguous memory management fit algorithms
	int NextFit(int request_size);
	int BestWorstFit(int size, std::string best_or_worst);
	int FreeSize(int index);

//PRIVATE VARIABLES
private:
	std::vector<char> memory;
	int mem_size;	
	int last_placement;
};

#endif