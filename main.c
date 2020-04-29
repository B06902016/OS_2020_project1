#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "scheduler.h"
#include "process_control.h"

int CMP(const void *a, const void *b){ // order in ready time, then name 
	Process *c = (Process *)a;
	Process *d = (Process *)b;
	
	if( c->ready_time < d->ready_time )
		return -1;
	else if( c->ready_time > d->ready_time )
		return 1;
	else 
		return strcmp(c->name, d->name);
}

/*
Function of main.c:
1. Read input
2. Sort by processes' ready_time
3. Call corresponding scheduler
*/

int main(){
	char policy[8];
	int n;
	scanf("%s", policy);
	scanf("%d", &n);
	Process* proc = malloc(n * sizeof(Process));

	for(int i = 0 ; i < n ; i++){
		Process *tmp = (proc + i);
		scanf("%s%d%d", tmp -> name, &(tmp -> ready_time), &(tmp -> exec_time));
		tmp -> pid = -1; // initialize pid
		if(pipe((tmp -> pipe_fd)) < 0){
			perror("error: pipe");
			exit(1);
		}
	}

	qsort(proc, n, sizeof(Process), CMP);
	assign_core(getpid(), SCHED_CORE);
	
#ifdef PRINT_PROC
	for(int i = 0 ; i < n ; i++)
		printf("%s %d %d\n", proc[i].name, proc[i].ready_time, proc[i].exec_time);
#endif

	if(strcmp(policy, "FIFO") == 0){
		scheduler_FIFO(proc, n);
	}
	else if(strcmp(policy, "RR") == 0){
		scheduler_RR(proc, n);
	}
	else if(strcmp(policy, "SJF") == 0){
		scheduler_SJF(proc, n);
	}
	else if(strcmp(policy, "PSJF") == 0){
		scheduler_PSJF(proc, n);
	}
	else{
		printf("No Such scheduler policy.\n");
	}

	return 0;
}
