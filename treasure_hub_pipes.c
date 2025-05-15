#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 2048
#define CMD_FILE "monitor_cmd.txt"

pid_t monitor_pid = -1;
int pipefd[2]; // pipefd[0] = read, pipefd[1] = write

void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitor is already running.\n");
        return;
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    monitor_pid = fork();
    if (monitor_pid == 0) {
        // Child: redirect stdout to pipe and exec monitor
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execl("./monitor", "monitor", NULL);
        perror("execl monitor");
        exit(EXIT_FAILURE);
    } else if (monitor_pid > 0) {
        // Parent: close write end
        close(pipefd[1]);
        printf("Monitor started (PID %d).\n", monitor_pid);
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

void stop_monitor() {
    if (monitor_pid > 0) {
        kill(monitor_pid, SIGTERM);
        waitpid(monitor_pid, NULL, 0);
        close(pipefd[0]);
        monitor_pid = -1;
        printf("Monitor stopped.\n");
    } else {
        printf("No monitor is running.\n");
    }
}

void send_command_and_read_output(const char *cmd_line) {
    // Write to monitor_cmd.txt
    FILE *cmd_fp = fopen(CMD_FILE, "w");
    if (!cmd_fp) {
        perror("fopen monitor_cmd.txt");
        return;
    }
    fprintf(cmd_fp, "%s\n", cmd_line);
    fclose(cmd_fp);

    // Send SIGUSR1
    kill(monitor_pid, SIGUSR1);

    // Read response from monitor (pipe)
    char buffer[1024];
    ssize_t nbytes;
    printf("--- Monitor response ---\n");
    while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[nbytes] = '\0';
        printf("%s", buffer);
        if (nbytes < sizeof(buffer) - 1)
            break; // assume done
    }
    printf("------------------------\n");
}

void interactive_loop() {
    char input[MAX_CMD_LEN];

    while (1) {
        printf("hub> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0'; // strip newline

        if (strcmp(input, "exit") == 0) {
            stop_monitor();
            break;
        } else if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strncmp(input, "list_hunts", 10) == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0 ||
                   strncmp(input, "calculate_score", 15) == 0) {
            if (monitor_pid <= 0) {
                printf("Monitor is not running.\n");
            } else {
                send_command_and_read_output(input);
            }
        } else {
            printf("Unknown command.\n");
        }
    }
}

int main() {
    printf("Treasure Hub Interface. Type 'start_monitor' to begin, 'exit' to quit.\n");
    interactive_loop();
    return 0;
}
