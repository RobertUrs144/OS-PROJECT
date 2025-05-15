#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "treasure_manager.h" // Assumes functions like listFilesInDirectory(), viewTreasureInFile(), log_op(), etc.

#define CMD_FILE "monitor_cmd.txt"
#define MAX_LINE 2048

void handle_sigusr1(int signo) {
    // Placeholder
}

void handle_sigterm(int signo) {
    // Placeholder
}

void handle_sigchld(int signo) {
    // Optional: handle cleanup or log child termination
    log_op(".", "Received SIGCHLD (child terminated)");
}

void process_command() {
    FILE *cmd_fp = fopen(CMD_FILE, "r");
    if (!cmd_fp) {
        perror("Failed to open monitor_cmd.txt");
        return;
    }

    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), cmd_fp)) {
        fclose(cmd_fp);
        return;
    }
    fclose(cmd_fp);

    line[strcspn(line, "\n")] = '\0';  // Remove newline

    // Tokenize command
    char *token = strtok(line, " ");
    if (!token) return;

    if (strcmp(token, "list_hunts") == 0) {
        log_op(".", "Received command: list_hunts");
        listFilesInDirectory();  // This should print to stdout

    } else if (strcmp(token, "list_treasures") == 0) {
        char *dir = strtok(NULL, " ");
        if (!dir) {
            printf("Error: list_treasures requires directory\n");
            return;
        }
        char msg[MAX_LINE];
        snprintf(msg, sizeof(msg), "Received command: list_treasures %s", dir);
        log_op(dir, msg);
        viewTreasureInFile(dir);  // Should also print to stdout

    } else if (strcmp(token, "view_treasure") == 0) {
        char *dir = strtok(NULL, " ");
        if (!dir) {
            printf("Error: view_treasure requires directory\n");
            return;
        }
        char msg[MAX_LINE];
        snprintf(msg, sizeof(msg), "Received command: view_treasure %s", dir);
        log_op(dir, msg);
        viewTreasureInFile(dir);  // Should also print to stdout

    } else if (strcmp(token, "calculate_score") == 0) {
        char *dir = strtok(NULL, " ");
        if (!dir) {
            printf("Error: calculate_score requires directory\n");
            return;
        }

        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        } else if (pid == 0) {
            // Child: redirect stdout to write end of pipe
            close(fd[0]); // Close read end
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);

            execl("./calculate_score", "calculate_score", dir, NULL);
            perror("execl");
            exit(1);
        } else {
            // Parent: read output from child
            close(fd[1]); // Close write end
            char buffer[1024];
            ssize_t n;
            while ((n = read(fd[0], buffer, sizeof(buffer)-1)) > 0) {
                buffer[n] = '\0';
                printf("%s", buffer);  // Forward to monitor's stdout (pipe to hub)
            }
            close(fd[0]);

            char msg[256];
            snprintf(msg, sizeof(msg), "Finished calculating score for '%s'", dir);
            log_op(dir, msg);
        }
    }
}

int main() {
    struct sigaction sa_usr1, sa_term, sa_chld;
    sigset_t sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGTERM);
    sigaddset(&sigset, SIGCHLD);

    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;

    sa_term.sa_handler = handle_sigterm;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;

    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction SIGUSR1");
        exit(1);
    }

    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(1);
    }

    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    log_op(".", "Monitor process started");

    while (1) {
        int sig;
        sigwait(&sigset, &sig);

        if (sig == SIGUSR1) {
            process_command();
        } else if (sig == SIGTERM) {
            log_op(".", "SIGTERM received. Monitor shutting down in 2 seconds...");
            sleep(2);
            log_op(".", "Monitor terminated.");
            break;
        } else if (sig == SIGCHLD) {
            handle_sigchld(sig);
        }
    }

    return 0;
}
