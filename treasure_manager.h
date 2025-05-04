#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MAX_LENGTH_NAME 2048

typedef struct {
    long double latitude;
    long double longitude;
} GPS;

typedef struct {
    int treasureID;
    char user_name[MAX_LENGTH_NAME];
    char clue[100];
    GPS gps;
    int value;
} Treasure;

/* Function Prototypes */
Treasure *addHunt_ID(int treasureID);
void create_symlink(const char *dir_name);
void log_op(const char *dir_name, const char *op);
void listFilesInDirectory();
void createDirectory(const char *dir_name);
void addTreasureToFile(const char *dir_name, Treasure *t);
void viewTreasureInFile(const char *dir_name);
void remove_hunt(const char *dir_name);
void remove_treasure(const char *dir_name);

#endif /* TREASURE_MANAGER_H */
