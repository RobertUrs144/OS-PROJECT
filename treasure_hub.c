#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t monitor_pid = -1;
int monitor_running = 0;

void handle_sigchld(int sig){

    int status;
    waitpid(monitor_pid, &status, 0);
    monitor_running = 0;
    printf("Monitor terminated with status %d\n", status);
}

void handle_usr1(int sig){

    printf("[Monitor] Received signal: SIGUSR1\n");
}

void handle_term(int sig){

    printf("[Monitor] Received SIGTERM, shutting down....\n");
    usleep(2000000);
    exit(0);
}

void start_monitor(){

    if(monitor_running){

        printf("Monitor is already running (PID %d)\n", monitor_pid);
        return;
    }

    monitor_pid = fork();

    if(monitor_pid < 0){

        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(monitor_pid == 0){//child monitor process

        struct sigaction sa_usr1;
        sa_usr1.sa_handler = handle_usr1;
        sigemptyset(&sa_usr1.sa_mask);
        sa_usr1.sa_flags = 0;
        sigaction(SIGUSR1, &sa_usr1, NULL);

        struct sigaction sa_term;
        sa_term.sa_handler = handle_term;
        sigemptyset(&sa_term.sa_mask);
        sa_term.sa_flags = 0;
        sigaction(SIGTERM, &sa_term, NULL);

        printf("[Monitor] Running (PID %d) Waiting for signals...\n", getpid());

        while(1){

            pause();
        }
    }

    monitor_running = 1; //parent
    printf("Monitor started with PID %d\n", monitor_pid);
}

void stop_monitor(){

    if(!monitor_running){

        printf("No monitor running\n");
        return;
    }

    kill(monitor_pid, SIGTERM);
    printf("Stop signal sent to monitor (PID %d)\n", monitor_pid);
}

int main(){

    struct sigaction sa_chld;

    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa_chld, NULL);

    start_monitor();

    sleep(1);
    kill(monitor_pid, SIGUSR1);

    sleep(1);
    stop_monitor();

    sleep(1);

    return 0;
}