#include <vector>
#include <utility>
#include <iostream>
#include <cassert>
#include <set>
#include "memory.h"

//Constructor
Memory::Memory(){
	mem_size = 256;
	for(int i=0; i<mem_size; ++i){
		memory.push_back('.');
	}
	last_placement = 0;
}

//non-default constructor
Memory::Memory(int mem_size_){
	mem_size = mem_size_;
	for(int i=0; i<mem_size; ++i){
		memory.push_back('.');
	}
	last_placement = 0;
}

//helper function
void Memory::MemoryPrint(int row_length){
	for(int i=0; i<row_length; ++i){
		std::cout<<'=';
	}
	for(int i=0; i<mem_size; ++i){
		if(i==mem_size||i%row_length==0){
			std::cout<<std::endl;
		}
		std::cout<<memory[i];
	}
	std::cout<<std::endl;
	for(int i=0; i<row_length; ++i){
		std::cout<<'=';
	}
	std::cout<<std::endl;
	return;
}	

//check the amount of free space in memory
int Memory::FreeSpace(){
	int count = 0;
	for(int i=0; i<mem_size; ++i){
		if(memory[i]=='.')count++;
	}
	return count;
}

//return 0 if given frame is not moved
//return 1 if given frame is successfully moved
int Memory::MoveFrame(int frame_index){
	if(memory[frame_index]=='.')return 0;
	int target_index;
	for(target_index=frame_index; 
		target_index>0; --target_index){
		if(memory[target_index-1]!='.')break;
	}
	if(target_index==frame_index)return 0;
	else{
		memory[target_index]=memory[frame_index];
		memory[frame_index]='.';
		return 1;
	}
}

//clear the memory of a exited process
int Memory::RemoveProcess(char pid){
	int count = 0;
	for(int i=0; i<mem_size; ++i){
		if(memory[i]==pid){
			memory[i]='.';
			count++;
		}
	}
	return count;
}

//add process -- contiguous
int Memory::AddProcessCont(char pid, int offset, int length){
	assert(offset+length<=mem_size);
	int i = offset;
	for(i=offset; i<offset+length; ++i){
		if(memory[i]=='.'){
			memory[i]=pid;
		}else{
			std::cerr<<"Memory::AddProcess(): Memory in use!\n";
			return -1;
		}
	}
	last_placement = i;
	return 0;
}

//add process -- non-contiguous
void Memory::AddProcessNonCont(char pid, int length){
	int count=0;
	for(int i=0; i<mem_size; ++i){
		if(memory[i]=='.'){
			memory[i]=pid;
			++count;
		}
		if(count == length)break;
	}
}


//defragmentation
int Memory::Defragmentation(std::set<char> & moved_processes){
	int count = 0;
	for(int i=0; i<mem_size; ++i){
		char c = memory[i];
		int n = MoveFrame(i);
		count+=n;
		if(n==1)moved_processes.insert(c);
	}
	return count;
}

//contiguous memory management fit algorithms
int Memory::NextFit(int request_size){
	//go to the last placed process tail
	int index_rec = last_placement;
	int index = -1;
	bool found = false;
	for(int i=index_rec; i<mem_size; ++i){
		if( memory[i]=='.'){
			int this_size = FreeSize(i);
			if(this_size>=request_size){
				index = i;
				found = true;
				break;
			}
			i+=this_size;
			i--;
		}
	}
	//after hitting bottom, start from top
	if(!found){
		for(int i=0; i<index_rec; ++i){
			if( (i==0||memory[i-1]!='.') && memory[i]=='.'){
				int this_size = FreeSize(i);
				if(this_size>=request_size){
					index = i;
					break;
				}
			}
		}	
	}
	return index;
}

int Memory::BestWorstFit(int request_size, 
						 std::string best_or_worst){
	int current_size;
	if(best_or_worst=="BEST")current_size = mem_size+1;
	else{
		current_size=0;
	}
	int index=0;
	//check free spaces from their starting positions
	for(int i=0; i<mem_size; ++i){
		if( (i==0||memory[i-1]!='.') && memory[i]=='.'){
			int this_size = FreeSize(i);
			if(best_or_worst=="BEST"){
				if(this_size>=request_size && this_size<current_size){
					index = i;
					current_size = this_size;
				}
			}else{
				if(this_size>=request_size && this_size>current_size){
					index = i;
					current_size = this_size;
				}
			}
		}
	}
	// no space available
	if(current_size==0||current_size==mem_size+1)return -1;
	return index;
}

//measure size of a partition
int Memory::FreeSize(int index){
	int answer=0;
	while(memory[index]=='.'&&index<mem_size){
		answer++;
		index++;
	}
	return answer;
}