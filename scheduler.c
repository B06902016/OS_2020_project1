#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include "scheduler.h"
#include "process_control.h"

int scheduler_FIFO(Process *proc, int N_procs){
	int total_time = 0;
	int cur = -1;

	while(1){
		cur += 1;
		// finished all process
		if(cur >= N_procs)		
			break;

		// process not yet ready
		while(proc[cur].ready_time > total_time){	
			TIME_UNIT();
			total_time += 1;
		}

		// execute child process
		pid_t chpid = proc_create(proc[cur]);
		proc_resume( chpid );

		// run until child process finishes
		while( proc[cur].exec_time > 0 ){
			// communicate with child, run one unit of time
			write(proc[cur].pipe_fd[1], "run", strlen("run"));
			TIME_UNIT();
			total_time += 1;
			proc[cur].exec_time -= 1;
		}		

		// wait for child
		waitpid(chpid, NULL, 0);

		// go to next round for next process
	}
	return 0;
}


int find_shortest(Process *proc, int N_procs, int cur_time){
	int shortest = -1, excute_time = INT_MAX;

	for (int i = 0; i < N_procs; i++){
		if (proc[i].ready_time <= cur_time && proc[i].pid == -1){
			proc[i].pid = proc_create(proc[i]);
		}
		if (proc[i].ready_time <= cur_time && proc[i].exec_time && proc[i].exec_time < excute_time){
			excute_time = proc[i].exec_time;
			shortest = i;
		}
	}

	return shortest;
}

int scheduler_SJF(Process *proc, int N_procs){
	int finish = 0;
	int total_time = 0;

#ifdef PRINT_LOG
	FILE *fp = fopen("./scheduler_log/SJF_5.out", "wb");
	char mesg[256] = "";
#endif

	while (finish < N_procs){
		int target = find_shortest(proc, N_procs, total_time);
		
		if (target != -1){
#ifdef PRINT_LOG
			sprintf(mesg, "process %s, start at %d\n", proc[target].name, total_time);
			fprintf(fp, "%s", mesg);
			fflush(fp);
#endif

			proc_resume( proc[target].pid );

			while (proc[target].exec_time > 0){
				// tell child process to run 1 time unit
				write(proc[target].pipe_fd[1], "run", strlen("run"));
				TIME_UNIT();
				total_time++;
				proc[target].exec_time--;
			}

			// wait child process
			waitpid(proc[target].pid, NULL, 0);
			
			// increase finished process by 1
			finish++;		
			
#ifdef PRINT_LOG
			sprintf(mesg, "process %s, end at %d\n", proc[target].name, total_time);
			fprintf(fp, "%s", mesg);
			fflush(fp);
#endif
		}

		else{
			TIME_UNIT();
			total_time++;
		}
	}

#ifdef PRINT_LOG
	fclose(fp);
#endif

	return 0;
}


int scheduler_PSJF(Process *proc, int N_procs){

	int total_time = 0, last_turn = -1;
	int finish = 0, started[N_procs];
	memset(started, 0, sizeof(started));

#ifdef PRINT_LOG
	FILE *fp = fopen("./scheduler_log/PSJF_5.out", "wb");
	char mesg[256] = "";
#endif
	
	while (finish < N_procs){
		int target = find_shortest(proc, N_procs, total_time);
		
		if (target != -1){
			started[target] = 1;
			proc_resume( proc[target].pid );
			last_turn = target;

			// tell child process to run 1 time unit
			write(proc[target].pipe_fd[1], "run", strlen("run"));
			TIME_UNIT();
			total_time++;

			proc[target].exec_time--;		
			proc_kickout( proc[target].pid );
			
			if (proc[target].exec_time == 0){		
				// wait child process
				waitpid(proc[target].pid, NULL, 0);
				finish++;
#ifdef PRINT_LOG
				sprintf(mesg, "process %s, end at %d\n", proc[target].name, time);
				fprintf(fp, "%s", mesg);
				fflush(fp);
#endif
			}
		}		
		else{
			TIME_UNIT();
			total_time++;
		}
	}

#ifdef PRINT_LOG
	fclose(fp);
#endif

	return 0;
}


int scheduler_RR(Process *proc, int N_procs){

	// pid_t chpids[N_procs] = {0};

	int finish = 0; //number of finished processes
	int total_time = 0; //current time

	while(1){

		int nt = 0; // number of processes not allowed to be executed
		int next_ex_t = INT_MAX; //the closest ready time from current time (used when nt == N_procs)

		for(int i=0; i<N_procs; i++){

			if( total_time < proc[i].ready_time ){
				if( proc[i].ready_time < next_ex_t ) 
					next_ex_t = proc[i].ready_time;
				nt ++;
				continue;
			}
			else if( proc[i].exec_time <= 0 ){
				nt ++;
				continue;
			} 

			if( proc[i].pid > 0 ){
				proc_resume( proc[i].pid );
			}
			else{ // if process hasn't been created
				//printf("creating process %s\n", proc[i].name);
				proc[i].pid = proc_create( proc[i] );
				proc_resume( proc[i].pid );
			}

			// run an RR round
			int kt = RR_SLICE; //time quantum for RR
			while( proc[i].exec_time > 0 && kt > 0){
				//if(kt == RR_SLICE) printf("ask process %s to run\n", proc[i].name);
				write(proc[i].pipe_fd[1], "run", strlen("run")); // tell process to run 1 time unit
				TIME_UNIT(); // run 1 time unit itself
				kt --;
				proc[i].exec_time --;
				total_time ++;
			}

			// if process finished
			if(proc[i].exec_time == 0){
				waitpid(proc[i].pid, NULL, 0);
				finish ++;
			}
			else
				proc_kickout( proc[i].pid );				
		}

		// all processes finished
		if( finish >= N_procs ) 
			break;

		if( nt >= N_procs){ // run itself when not finished and no process can be executed
			while( total_time < next_ex_t ){ //until at least a process is ready
				TIME_UNIT();
				total_time ++;
			}
		}
	}

	return 0;
}
