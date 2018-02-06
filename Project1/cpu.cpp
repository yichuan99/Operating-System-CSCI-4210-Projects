#include <vector>
#include <iostream>
#include "cpu.h"

//CONSTRUCTORS
CPU::CPU(){
	t_cs = 8; 
	t_RR = 84; 
	cores = std::vector<bool> (1,false);
	counter = 0;
	idle = true;
	switch_over = -1;
	rr_counter = 0;
}

CPU::CPU(	int t_cs_, int t_RR_, int num_core){
	t_cs = t_cs_;
	t_RR = t_RR_;
	cores = std::vector<bool> (num_core, false);
	counter = 0;
	idle = true;
	switch_over = -1;
	rr_counter = 0;
}

//Burst process
void CPU::burst(){
	current_process.CountDownBurst();
}

//Load process
void CPU::Load(Process p){
	current_process = p;
	switch_over = counter+t_cs/2;
}

//unload process
void CPU::unLoad(){
	unloading = true;
	switch_over = counter+t_cs/2;
}

//in switch mode
bool CPU::inSwitch(){
	if(counter>=switch_over){
		unloading = false;
		return false;
	}
	return true;
}
