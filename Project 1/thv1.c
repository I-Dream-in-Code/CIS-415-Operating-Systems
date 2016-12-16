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
#include "p1fxns.h"


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

int main(int argc, char *argv[])
{
    int N_PROCESSES;

    char *COMMAND;
    char **command_args;
    int pid;
    int has_remaining_args = 0;
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
      
        COMMAND = malloc(sizeof(char ) *1024);
		COMMAND=p1strdup(&argv[1][10]);
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

    //if both --number and --processors given
    else if (p1strneq(argv[1], "--n", 3) && p1strneq(argv[2], "--p", 3))
    {
        N_PROCESSES = p1atoi(&argv[1][p1strlen(argv[1]) - 1]);
       
        COMMAND = p1strdup(&argv[3][10]);
        if (argv[4] != NULL)
        {
            has_remaining_args = 1;
            command_args = malloc(sizeof(char *) * (argc - 4 + 1));
            for (int i = 3; i < argc; i++){
                command_args[i - 3] = p1strdup(argv[i]);
			}
			  command_args[argc] = NULL;
        }
    }
    int pid_array[N_PROCESSES];
    struct timeval *start = malloc(sizeof(struct timeval));
    struct timezone *start_t = malloc(sizeof(struct timezone));
    gettimeofday(start, start_t);
	
	//run n processes with COMMAND using exec
    for (int i = 0; i < N_PROCESSES; i++)
    {
		 pid=fork();
        if (pid < 0)
            p1perror(2, "cannot fork");
        else if (pid == 0)
        {
         
            if (has_remaining_args==1)
                execvp(COMMAND, command_args);
            else
            {
				//if no additional arguments are given
				command_args=malloc(sizeof(char *)*1);
				command_args[0]=NULL;
                execvp(COMMAND, command_args);
            }
        }
        else{
			pid_array[i]=pid;
		}
    }
    for (int i = 0; i < N_PROCESSES; i++)
    {
        int status;
        waitpid(pid_array[i], &status, 0);
    }
    struct timeval *stop = malloc(sizeof(struct timeval));;
    gettimeofday(stop, start_t);
    int result_s = stop->tv_sec - start->tv_sec;
    int result_u = stop->tv_usec - start->tv_usec;
    char *result = malloc(sizeof(char) * 1024);
    char *result_u_s = malloc(sizeof(char) * 1024);
    char *result_s_s = malloc(sizeof(char) * 1024);
    p1itoa(result_u, result_u_s);
    p1itoa(result_s, result_s_s);
    char *n_processes_s = malloc(sizeof(char) * 1024);
	
	//switch for stop time microseconds to fraction
    switch (digit_count(result_u))
    {
        case 6:
            p1strcat(result, result_s_s);
            p1strcat(result, ".");
            p1strcat(result, result_u_s);
			break;
        case 5:
            p1strcat(result, result_s_s);
            p1strcat(result, ".0");
            p1strcat(result, result_u_s);
			break;
        case 4:
            p1strcat(result, result_s_s);
            p1strcat(result, ".00");
            p1strcat(result, result_u_s);
			break;
        case 3:
            p1strcat(result, result_s_s);
            p1strcat(result, ".000");
            p1strcat(result, result_u_s);
			break;
        case 2:
            p1strcat(result, result_s_s);
            p1strcat(result, ".0000");
            p1strcat(result, result_u_s);
			break;
        case 1:
            p1strcat(result, result_s_s);
            p1strcat(result, ".00000");
            p1strcat(result, result_u_s);
			break;
    }
    
    //print the elapsed time N_PROCESSES copies of COMMAND is *result* seconds
    p1itoa(N_PROCESSES, n_processes_s);
    p1itoa(result_s, result);
    p1itoa(result_u, result_u_s);
    p1strcat(result, ".");
    p1strcat(result, result_u_s);
    p1putstr(2, "The elapsed time to execute ");
    p1putstr(2, n_processes_s);
    p1putstr(2, " copies of ");
    p1putstr(2, COMMAND);
    p1putstr(2, " is ");
    p1putstr(2, result);
    p1putstr(2, " seconds.\n");
	
	//free
	
    free(result_u_s);
    free(result_s_s);
    free(result);
    free(n_processes_s);
	free(COMMAND);
	free(start);
	free(stop);
	free(start_t);
	if(has_remaining_args)
    free(command_args);
}
