#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define CMD_FILE "monitor_cmd.txt"
#define MAX_INPUT 1024

pid_t monitor_pid = -1;

void write_command(const char *command) {
    FILE *fp = fopen(CMD_FILE, "w");
    if (!fp) {
        perror("Failed to write command");
        return;
    }
    fprintf(fp, "%s\n", command);
    fclose(fp);
}

void send_sigusr1() {
    if (monitor_pid > 0) {
        kill(monitor_pid, SIGUSR1);
    } else {
        printf("Monitor not running.\n");
    }
}

void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitor already running with PID %d\n", monitor_pid);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork monitor");
        return;
    }

    if (pid == 0) {
        // Child process runs the monitor
        execl("./monitor", "monitor", NULL);
        perror("Failed to exec monitor");
        exit(1);
    } else {
        monitor_pid = pid;
        printf("Monitor started with PID %d\n", monitor_pid);
        sleep(1); // Let the monitor initialize
    }
}

void stop_monitor() {
    if (monitor_pid <= 0) {
        printf("Monitor is not running.\n");
        return;
    }

    kill(monitor_pid, SIGTERM);
    waitpid(monitor_pid, NULL, 0); // Wait for monitor to finish
    printf("Monitor with PID %d stopped.\n", monitor_pid);
    monitor_pid = -1;
}

void cleanup_and_exit() {
    if (monitor_pid > 0) {
        kill(monitor_pid, SIGTERM);
        waitpid(monitor_pid, NULL, 0);
        printf("Monitor terminated on exit.\n");
    }
    printf("Exiting treasure_hub.\n");
    exit(0);
}

int main() {
    char input[MAX_INPUT];

    printf("Welcome to the Treasure Hub!\n");

    while (1) {
        printf("\nAvailable commands:\n");
        printf("  start_monitor\n");
        printf("  list_hunts\n");
        printf("  list_treasures <directory>\n");
        printf("  view_treasure <directory>\n");
        printf("  calculate_score <directory>\n");
        printf("  stop_monitor\n");
        printf("  exit\n");

        printf("\n> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("Input error. Exiting.\n");
            break;
        }

        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(input, "exit") == 0) {
            cleanup_and_exit();
        } else if (strncmp(input, "list_hunts", 10) == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0) {
            if (monitor_pid <= 0) {
                printf("Error: Monitor is not running. Use start_monitor first.\n");
                continue;
            }
            write_command(input);
            send_sigusr1();
        }else if(strncmp(input, "calculate_score", 15) == 0){
            if(monitor_pid <= 0){

                printf("Error: Monitor is not running. Start the monitor first\n");
                continue;
            }

            write_command(input);
            send_sigusr1();

        }else {
            printf("Invalid command.\n");
        }
    }

    cleanup_and_exit();
    return 0;
}
