/*
Michael Harris
Harris
CIS 415 Project 1
I collaborated with Ian Garrett
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "p1fxns.h"

char *COMMAND;
char **command_args;
int has_remaining_args = 0;
int N_PROCESSES;
int N_PROCESSORS;
int ** pid_array;
int sig_child_counter=0;
int i=0;

int digit_count(int n)
{
    int count = 0;
    while (n != 0)
    {
        n = n / 10;
        count++;
    }
    return count;
}

void sigusr1()
{

    if (has_remaining_args == 1)
        execvp(COMMAND, command_args);
    else
    {
        //if no additional arguments are given
        command_args = malloc(sizeof(char *) * 2);
        char * cmd[] = {"."};
        for (int i = 0; i < 1; i++)
            command_args[i] = cmd[i];
        command_args[1] = NULL;
        execvp(COMMAND, command_args );
    }
}

void sigalarm(){
	
	for(int j=0;j<N_PROCESSES;j++)
	kill(pid_array[i][j],SIGSTOP);
	for(int j=0;j<N_PROCESSES;j++){
		if(i+1==N_PROCESSORS)
	kill(pid_array[0][j],SIGCONT);
	else kill(pid_array[i+1][j],SIGCONT);}
	
}

void sigchild(){
	sig_child_counter+=1;
}
int main(int argc, char *argv[])
{

    int pid;
    signal(SIGUSR1, sigusr1);
	signal(SIGCHLD,sigchild);
		signal(SIGALRM,sigalarm);

    // input checking
    //if --number and --processors not given use env
    if ((p1strneq(argv[1], "--n", 3) == 0) && (p1strneq(argv[2], "--p", 3) == 0))
    {
        if (getenv("TH_NPROCESSES"))
            N_PROCESSES = p1atoi(getenv("TH_NPROCESSES"));
        else
        {
            p1putstr(2, " ERROR CODE 22:Invalid Argument:TH_NPROCESSES not set\n");
            return 22;
        }
        if (getenv("TH_NPROCESSORS"))
            N_PROCESSORS = p1atoi(getenv("TH_NPROCESSORS"));
        else
        {
            p1putstr(2, "ERROR CODE 22:Invalid Argument:TH_NPROCESSORS not set\n");
            return 22;
        }
        COMMAND = malloc(sizeof(char ) * 1024);
        COMMAND = p1strdup(&argv[1][10]);
        if (argv[2] != NULL)
        {
            has_remaining_args = 1;
            command_args = malloc(sizeof(char *) * (argc - 2 + 1));
            for (int i = 1; i < argc; i++)
                command_args[i - 1] = p1strdup(argv[i]);
            command_args[argc] = NULL;
        }
    }
    //if --number given but not --processors
    else if (p1strneq(argv[1], "--n", 3) && (p1strneq(argv[2], "--p", 3)) == 0)
    {
        if (getenv("TH_NPROCESSORS"))
        {
            N_PROCESSES = p1atoi(argv[1]);
            N_PROCESSORS = p1atoi(getenv("TH_NPROCESSORS"));
            COMMAND = p1strdup(&argv[2][10]);
        }
        else
        {
            p1putstr(2, "ERROR CODE 22:Invalid Argument: TH_NPROCESSORS not set\n");
            return 22;
        }
        if (argv[3] != NULL)
        {
            has_remaining_args = 1;
            command_args = malloc(sizeof(char *) * (argc - 3 + 1));
            for (int i = 2; i < argc; i++)
                command_args[i - 2] = p1strdup(argv[i]);
            command_args[argc] = NULL;
        }
    }
    //if --processors given but not number
    else if (p1strneq(argv[1], "--p", 3))
    {
        N_PROCESSORS = p1atoi(argv[1]);
        if (getenv("TH_NPROCESSES"))
        {
            N_PROCESSES = p1atoi(getenv("TH_NPROCESSES"));
            COMMAND = p1strdup(&argv[2][10]);
        }
        else
        {
            p1putstr(2, "ERROR CODE 22:Invalid Argument:TH_NPROCESSES not set\n");
            return 22;
        }
        if (argv[4] != NULL)
        {
            has_remaining_args = 1;
            command_args = malloc(sizeof(char *) * (argc - 3 + 1));
            for (int i = 2; i < argc; i++)
                command_args[i - 2] = p1strdup(argv[i]);
            command_args[argc] = NULL;
        }
    }
    //if both --number and --processors given
    else if (p1strneq(argv[1], "--n", 3) && p1strneq(argv[2], "--p", 3))
    {
        N_PROCESSES = p1atoi(&argv[1][9]);
        N_PROCESSORS = p1atoi(&argv[2][13]);
        COMMAND = p1strdup(&argv[3][10]);
        if (argv[4] != NULL)
        {
            has_remaining_args = 1;
            command_args = malloc(sizeof(char *) * (argc - 4 + 1));
            for (int i = 3; i < argc; i++)
                command_args[i - 3] = p1strdup(argv[i]);
            command_args[argc] = NULL;
        }
    }

    pid_array = malloc(sizeof(int *)*N_PROCESSORS);
    for (int i = 0; i < N_PROCESSORS; i++)
        pid_array[i] = malloc(sizeof(int) * N_PROCESSES);
    //fork and pause child, add pid to array from parent
    for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++)
        {
           pid=fork();
            if (pid < 0)
                p1perror(2, "cannot fork");
            else if (pid == 0)
            {
                pause();
            }
            else{
				 pid_array[i][j] = pid;
			}
        }
    }


    for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++){
            kill(pid_array[i][j], SIGUSR1);
			}
    }
    
    //set timer to 1000 microseconds
    //each alarm stops  current processor
    //starts next
    //until all processes have sent SIGCHLD
do{
struct itimerval new;

struct timezone new_t;	
new.it_interval.tv_usec=1000;
gettimeofday(&new.it_value,&new_t);
setitimer(ITIMER_REAL,&new,NULL);


	if(i+1==N_PROCESSORS) i=0;
	else i=i+1;
	
	
}while(sig_child_counter!=(N_PROCESSES*N_PROCESSORS)-1);
	

	//graceful wait
	for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++){
            int status;
		waitpid(pid_array[i][j],&status,0);
    }}

//     struct timeval *stop = malloc(sizeof(struct timeval));;
//     gettimeofday(stop, start_t);
//     int result_s = stop->tv_sec - start->tv_sec;
//     int result_u = stop->tv_usec - start->tv_usec;
//     char *result = malloc(sizeof(char) * 1024);
//     char *result_u_s = malloc(sizeof(char) * 1024);
//     char *result_s_s = malloc(sizeof(char) * 1024);
//     p1itoa(result_u, result_u_s);
//     p1itoa(result_s, result_s_s);
//     char *n_processes_s = malloc(sizeof(char) * 1024);
// //switch for stop time microseconds to fraction
//     switch (digit_count(result_u))
//     {
//         case 6:
//             p1strcat(result, result_s_s);
//             p1strcat(result, ".");
//             p1strcat(result, result_u_s);
//         case 5:
//             p1strcat(result, result_s_s);
//             p1strcat(result, ".0");
//             p1strcat(result, result_u_s);
//         case 4:
//             p1strcat(result, result_s_s);
//             p1strcat(result, ".00");
//             p1strcat(result, result_u_s);
//         case 3:
//             p1strcat(result, result_s_s);
//             p1strcat(result, ".000");
//             p1strcat(result, result_u_s);
//         case 2:
//             p1strcat(result, result_s_s);
//             p1strcat(result, ".0000");
//             p1strcat(result, result_u_s);
//         case 1:
//             p1strcat(result, result_s_s);
//             p1strcat(result, ".00000");
//             p1strcat(result, result_u_s);
//     }
// //print the elapsed time N_PROCESSES copies of COMMAND is *result* seconds
//     p1itoa(N_PROCESSES, n_processes_s);
//     p1itoa(result_s, result);
//     p1itoa(result_u, result_u_s);
//     p1strcat(result, ".");
//     p1strcat(result, result_u_s);
//     p1putstr(2, "The elapsed time to execute ");
//     p1putstr(2, n_processes_s);
//     p1putstr(2, " copies of ");
//     p1putstr(2, COMMAND);
//     p1putstr(2, " is ");
//     p1putstr(2, result);
//     p1putstr(2, " seconds.");
//free
//     free(result_u_s);
//     free(result_s_s);
//     free(result);
//     free(n_processes_s);
    free(COMMAND);
//     free(start);
//     free(stop);
//     free(start_t);
for(int i=0;i<N_PROCESSORS;i++){
	free(pid_array[i]);
	
}
free(pid_array);
    if (has_remaining_args)
        free(command_args);
}
