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
#include <sys/stat.h>
#include <fcntl.h>
#include "p1fxns.h"

char *COMMAND;
char **command_args;
int has_remaining_args = 0;
int N_PROCESSES;
int N_PROCESSORS;
int ** pid_array;
int sig_child_counter = 0;
int i = 0;
int *** pid_files;



char * PROC_PATH = "/proc/";
char * SCHED_PATH = "/sched";
char * ENVIRON_PATH = "/environ";
char * EXE_PATH = "/exe";
char * MEM_PATH = "/mem";
char * STATUS_PATH = "/status";
char * INTERRUPTS_PATH = "/interrupts";
char * IO_PORTS_PATH = "/io";
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

void sigalarm()
{
    for (int j = 0; j < N_PROCESSES; j++)
        kill(pid_array[i][j], SIGSTOP);
    for (int j = 0; j < N_PROCESSES; j++)
    {
        if (i + 1 == N_PROCESSORS)
            kill(pid_array[0][j], SIGCONT);
        else kill(pid_array[i + 1][j], SIGCONT);
    }
}

void sigchild()
{
    sig_child_counter += 1;
}
int main(int argc, char *argv[])
{
    int pid;
    signal(SIGUSR1, sigusr1);
    signal(SIGCHLD, sigchild);
    signal(SIGALRM, sigalarm);
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
    pid_files = malloc(sizeof(int *)*N_PROCESSORS);
    for (int i = 0; i < N_PROCESSORS; i++)
        pid_files[i] = malloc(sizeof(int*) * N_PROCESSES);
    for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++)
            pid_files[i][j] = malloc(sizeof(int) * 6);
    }
    //fork and pause child, add pid to array from parent
    for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++)
        {
            pid = fork();
            if (pid < 0)
                p1perror(2, "cannot fork");
            else if (pid == 0)
                pause();
            else
            {
                char * pid_s = malloc(sizeof(char) * 1024);
                p1itoa(pid, pid_s);
                char * pid_sched;
                pid_sched = p1strdup(PROC_PATH);
                p1strcat(pid_sched, pid_s);
                p1strcat(pid_sched, SCHED_PATH);
                char * pid_environ;
                pid_environ = p1strdup(PROC_PATH);
                p1strcat(pid_environ, pid_s);
                p1strcat(pid_environ, ENVIRON_PATH);
                char * pid_exe;
                pid_exe = p1strdup(PROC_PATH);
                p1strcat(pid_exe, pid_s);
                p1strcat(pid_exe, EXE_PATH);
                char * pid_mem ;
                pid_mem = p1strdup(PROC_PATH);
                p1strcat(pid_mem, pid_s);
                p1strcat(pid_mem, MEM_PATH);
                char * pid_status;
                pid_status = p1strdup(PROC_PATH);
                p1strcat(pid_status, pid_s);
                p1strcat(pid_status, STATUS_PATH);
                char * pid_io;
                pid_io = p1strdup(PROC_PATH);
                p1strcat(pid_io, pid_s);
                p1strcat(pid_io, IO_PORTS_PATH);
                pid_array[i][j] = pid;
                pid_files[i][j][0] = open(pid_sched, O_RDONLY);
                pid_files[i][j][1] = open(pid_environ, O_RDONLY);
                pid_files[i][j][2] = open(pid_exe, O_RDONLY);
                pid_files[i][j][3] = open(pid_mem, O_RDONLY);
                pid_files[i][j][4] = open(pid_status, O_RDONLY);
                pid_files[i][j][5] = open(pid_io, O_RDONLY);
                free(pid_sched);
                free(pid_environ);
                free(pid_exe);
                free(pid_mem);
                free(pid_status);
                free(pid_io);
                free(pid_s);
            }
        }
    }
    for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++)
        {
            kill(pid_array[i][j], SIGUSR1);
            continue;
        }
    }
    //set timer to 1000 microseconds
    //each alarm stops  current processor
    //starts next
    //until all processes have sent SIGCHLD
    while (sig_child_counter != (N_PROCESSES * N_PROCESSORS))
    {
		 for (int j = 0; j < N_PROCESSES; j++)
        {
            for (int k = 0; k < 5; k++)
            {
				p1putstr(1,"PROCESS ID: ");
				p1putint(1,pid_array[i][j]);
				p1putstr(1,"\n=====================\n");
				switch (k){
					case 0:
						p1putstr(1,"SCHED\n===============\n");
						break;
					case 1:
						p1putstr(1,"ENVIRON\n===============\n");
						break;
						case 2:
						p1putstr(1,"EXE\n===============\n");
						break;
						case 3:
						p1putstr(1,"MEM\n===============\n");
						break;
						case 4:
						p1putstr(1,"STATUS\n===============\n");
						break;
						case 5:
						p1putstr(1,"IO\n===============\n");
						break;
						
				}		
                char * buffer = malloc(sizeof(char) * 2048);
                read(pid_files[i][j][k],buffer,2048-1);
                
                    p1putstr(1, buffer);
                    lseek(pid_files[i][j][k],SEEK_SET,0);
                
                free(buffer);
            }
        }
        struct itimerval new;
        struct timezone new_t;
        new.it_interval.tv_usec = 1000;
        gettimeofday(&new.it_value, &new_t);
        setitimer(ITIMER_REAL, &new, NULL);
        if (i + 1 == N_PROCESSORS) i = 0;
        else i = i + 1;
       
    }
    //graceful wait
    for (int i = 0; i < N_PROCESSORS; i++)
    {
        for (int j = 0; j < N_PROCESSES; j++)
        {
            int status;
            waitpid(pid_array[i][j], &status, 0);
        }
    }

    free(COMMAND);

    for (int i = 0; i < N_PROCESSORS; i++)
        free(pid_array[i]);
    free(pid_array);
	for(int i=0;i<N_PROCESSORS;i++){
		for(int j=0;j<N_PROCESSES;j++){
			free(pid_files[i][j]);
		}
		free(pid_files[i]);
	}
	free(pid_files);
    if (has_remaining_args)
        free(command_args);
}
