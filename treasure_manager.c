#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>   // close, read, write, rmdir, unlink, symlink
#include <time.h>
#include "treasure_manager.h"

#define MAX_LENGTH_NAME 2048

// typedef struct {
//     long double latitude;
//     long double longitude;
// } GPS;

// typedef struct {
//     int treasureID;
//     char user_name[MAX_LENGTH_NAME];
//     char clue[100];
//     GPS gps;
//     int value;
// } Treasure;

Treasure *addHunt_ID(int treasureID) {
    Treasure *temp = (Treasure *)malloc(sizeof(Treasure));
    if (!temp) {
        perror("Memory allocation failed");
        exit(1);
    }
    temp->treasureID = treasureID;
    return temp;
}

void create_symlink(const char *dir_name) {
    char log_file[MAX_LENGTH_NAME];
    snprintf(log_file, sizeof(log_file), "%s/logged_hunt.txt", dir_name);

    char symlink_name[MAX_LENGTH_NAME];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", dir_name);

    if (symlink(log_file, symlink_name) == -1) {
        perror("Error while creating symlink");
    } else {
        printf("Symbolic link created: %s -> %s\n", symlink_name, log_file);
    }
}

void log_op(const char *dir_name, const char *op) {
    time_t raw_time;
    struct tm *timeInfo;
    char time_str[MAX_LENGTH_NAME];

    time(&raw_time);
    timeInfo = localtime(&raw_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeInfo);

    char entry[MAX_LENGTH_NAME];
    // Check the total length before formatting
    if (snprintf(entry, sizeof(entry), "[%s] %s\n", time_str, op) >= sizeof(entry)) {
        fprintf(stderr, "Warning: log entry truncated in log_op function.\n");
    }

    char path[MAX_LENGTH_NAME];
    snprintf(path, sizeof(path), "%s/logged_hunt.txt", dir_name);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1) {
        perror("Error while opening the log file");
        exit(1);
    }

    if (write(fd, entry, strlen(entry)) == -1) {
        perror("Error while writing to log file");
        close(fd);
        exit(1);
    }

    close(fd);
}


void listFilesInDirectory() {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    long total_size = 0;

    dir = opendir(".");
    if (!dir) {
        perror("opendir");
        exit(1);
    }

    printf("Listing files from the current directory:\n");
    printf("=========================================================================================\n");

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (stat(entry->d_name, &file_stat) == -1) {
            perror("stat");
            closedir(dir);
            exit(1);
        }

        total_size += file_stat.st_size;

        char time_str[MAX_LENGTH_NAME];
        struct tm *mod_time = localtime(&file_stat.st_mtime);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", mod_time);

        printf("File: %-25s    Size: %-10ld    Last Modified: %s\n",
               entry->d_name, file_stat.st_size, time_str);
    }

    printf("=========================================================================================\n");
    printf("Total size of all files: %ld bytes\n", total_size);
    closedir(dir);
}

void createDirectory(const char *dir_name) {
    int result = mkdir(dir_name, 0777); // Linux version with permissions

    if (result == -1) {
        if (errno == EEXIST) {
            printf("Directory '%s' already exists.\n", dir_name);
        } else {
            perror("mkdir failed");
        }
    } else {
        printf("Directory '%s' created successfully.\n", dir_name);
    }
}

void addTreasureToFile(const char *dir_name, Treasure *t) {
    char path[MAX_LENGTH_NAME];
    snprintf(path, sizeof(path), "%s/treasure.bin", dir_name);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1) {
        perror("Error while opening the treasure file");
        exit(1);
    }

    if (write(fd, t, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("Error while writing treasure");
        close(fd);
        exit(1);
    }

    printf("Treasure added to file: %s\n", path);
    close(fd);

    // Safely format the log message
    char message[MAX_LENGTH_NAME];
    char user_name[MAX_LENGTH_NAME];
    char clue[100];

    // Ensure 'user_name' and 'clue' do not exceed MAX_LENGTH_NAME
    strncpy(user_name, t->user_name, sizeof(user_name) - 1);
    user_name[sizeof(user_name) - 1] = '\0'; // Null terminate if it gets truncated

    strncpy(clue, t->clue, sizeof(clue) - 1);
    clue[sizeof(clue) - 1] = '\0'; // Null terminate if it gets truncated

    int ret = snprintf(message, sizeof(message), "ADD treasure%d by user=%s at %.4Lf, %.4Lf, value=%d",
             t->treasureID, user_name, t->gps.latitude, t->gps.longitude, t->value);
    if (ret >= sizeof(message)) {
        fprintf(stderr, "Warning: log message truncated in addTreasureToFile function.\n");
    }

    log_op(dir_name, message);
}

void viewTreasureInFile(const char *dir_name) {
    char path[MAX_LENGTH_NAME];
    snprintf(path, sizeof(path), "%s/treasure.bin", dir_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error while opening the treasure file");
        exit(1);
    }

    Treasure treasure;
    ssize_t bytesRead;

    printf("Content of %s:\n", path);
    printf("=====================================\n");

    while ((bytesRead = read(fd, &treasure, sizeof(Treasure))) > 0) {
        if (bytesRead != sizeof(Treasure)) {
            printf("Error: Incomplete data read\n");
            exit(1);
        }

        printf("Treasure ID: %d\n", treasure.treasureID);
        printf("User: %s\n", treasure.user_name);
        printf("Latitude: %Lf\n", treasure.gps.latitude);
        printf("Longitude: %Lf\n", treasure.gps.longitude);
        printf("Clue: %s\n", treasure.clue);
        printf("Value: %d\n", treasure.value);
        printf("=====================================\n");
    }

    if (bytesRead == -1) {
        perror("Error while reading treasure file");
    }

    close(fd);

    char message[MAX_LENGTH_NAME];
    snprintf(message, sizeof(message), "VIEW treasure%d", treasure.treasureID);
    log_op(dir_name, message);
}

void remove_hunt(const char *dir_name) {
    DIR *dir = opendir(dir_name);
    struct dirent *entry;
    char path[MAX_LENGTH_NAME];

    if (!dir) {
        perror("opendir");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
        struct stat path_stat;

        if (stat(path, &path_stat) == -1) {
            perror("stat failed");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            remove_hunt(path); // Recursive removal
        } else {
            if (unlink(path) != 0) {
                perror("Failed to delete file");
            }
        }
    }

    closedir(dir);

    if (rmdir(dir_name) == -1) {
        perror("rmdir");
        exit(1);
    } else {
        printf("Hunt directory '%s' removed successfully\n", dir_name);
    }

    char log_message[MAX_LENGTH_NAME];
    snprintf(log_message, sizeof(log_message), "REMOVE_HUNT %s", dir_name);
    log_op(dir_name, log_message);
}

void remove_treasure(const char *dir_name) {
    char path[MAX_LENGTH_NAME];
    snprintf(path, sizeof(path), "%s/treasure.bin", dir_name);

    if (unlink(path) == -1) {
        perror("Failed to delete treasure.bin file");
        exit(1);
    }

    printf("treasure.bin removed from '%s'\n", dir_name);

    char log_message[MAX_LENGTH_NAME];
    snprintf(log_message, sizeof(log_message), "REMOVE treasure");
    log_op(dir_name, log_message);
}

// int main(int argc, char **argv) {
//     if (argc < 2) {
//         printf("Error: Not enough arguments\n");
//         return 1;
//     }
//
//     if (strcmp(argv[1], "--add") == 0) {
//         if (argc < 3) {
//             printf("Error: Provide a directory name for --add\n");
//             return 1;
//         }
//
//         const char *dir_name = argv[2];
//         createDirectory(dir_name);
//
//         Treasure *treasure = addHunt_ID(1);
//         strcpy(treasure->user_name, "Sara");
//         treasure->gps.latitude = 45.1234;
//         treasure->gps.longitude = 25.3333;
//         strcpy(treasure->clue, "Behind the waterfall");
//         treasure->value = 100;
//
//         addTreasureToFile(dir_name, treasure);
//         free(treasure);
//
//         create_symlink(dir_name);
//
//     } else if (strcmp(argv[1], "--remove_hunt") == 0) {
//         if (argc < 3) {
//             printf("Error: Provide a directory name for --remove_hunt\n");
//             return 1;
//         }
//
//         remove_hunt(argv[2]);
//
//     } else if (strcmp(argv[1], "--remove_treasure") == 0) {
//         if (argc < 3) {
//             printf("Error: Provide a directory name for --remove_treasure\n");
//             return 1;
//         }
//
//         remove_treasure(argv[2]);
//
//     } else if (strcmp(argv[1], "--list") == 0) {
//         listFilesInDirectory();
//
//     } else if (strcmp(argv[1], "--view") == 0) {
//         if (argc < 3) {
//             printf("Error: Provide a directory name for --view\n");
//             return 1;
//         }
//
//         viewTreasureInFile(argv[2]);
//
//     } else {
//         printf("Unknown command. Use: --add, --remove_hunt, --remove_treasure, --list, --view\n");
//     }
//
//     return 0;
// }
