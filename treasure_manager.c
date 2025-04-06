#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h> //mkdir
#include <fcntl.h>
#include <unistd.h> //close, read, write
#include <time.h> //for lstat

#define MAX_LENGTH_NAME 1000 //max value for the arrays

typedef struct{ //struct for gps coordinates

    long double latitude;
    long double longitude;
}GPS;

typedef struct { //struct for the treasure data
    int treasureID;
    char user_name[MAX_LENGTH_NAME];
    char clue[100];
    GPS gps;
    int value;
}Treasure;


Treasure *addHunt_ID(int treasureID) {//function to add the treasure id

    Treasure *temp = (Treasure *)malloc(sizeof(Treasure));
    if (!temp) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    temp->treasureID = treasureID;
    return temp;
}

void listFilesInDirectory() {//function to list all the files in the current directory, with all the data about files
   
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    long total_size = 0;
    
    dir = opendir(".");
    if (dir == NULL) {//if the directory is empty, then show a message
        perror("opendir");
        exit(1);
    }

    printf("Listing files from the current directory:\n");
    printf("=========================================================================================\n");

    while ((entry = readdir(dir)) != NULL) {
        
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){

            continue; //skips . and ..
        }

        if(stat(entry->d_name, &file_stat) == -1){

            perror("stat\n");
            closedir(dir);
            exit(1);
        }

        total_size = total_size + file_stat.st_size; //.st_size is the size of file in bytes

        char time_str[MAX_LENGTH_NAME];
        
        struct tm *mod_time = localtime(&file_stat.st_mtime); //st_mtime is the recent time at which the file was modified
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", mod_time);

        printf("File: %-25s    Size: %-10ld    Last Modified: %s\n", entry->d_name, file_stat.st_size, time_str);

    }

    printf("=========================================================================================\n");
    printf("Total size of all files: %ld bytes\n", total_size);

    closedir(dir);
}

/*
add <hunt_id> //done
list <hunt_id> //done
view <hunt_id> <id> //done
remove_treasure <hunt_id> <id> //not sure how to do it
remove_hunt <hunt_id> //done
*/

//function to create a directory, in this case Hunt
void createDirectory(const char *dir_name){

    int result;

    result = _mkdir(dir_name);

    if(result == -1){

        if(errno = EEXIST){

            printf("Directory '%s' already exists.\n", dir_name);
        }else{

            perror("mkdir failed");
        }
    }else{

        printf("Directory '%s' created successfully.\n", dir_name);
    }
}

//function to append a treasure to a binary file inside directory Hunt
void addTreasureToFile(const char *dir_name, Treasure *t){

    char path[MAX_LENGTH_NAME];
    snprintf(path, sizeof(path), "%s/treasure.bin", dir_name);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);

    if(fd == -1){

        perror("Error while opening the treasure file\n");
        exit(1);
    }

    if(write(fd, t, sizeof(Treasure)) != sizeof(Treasure)){

        perror("Error while writing treasure\n");
        close(fd);
        exit(1);
    }

    printf("Treasure : %s\n", path);
    close(fd);
}

//function to display all treasures from a specific treasure.bin file
void viewTreasureInFile(const char *dir_name){

    char path[MAX_LENGTH_NAME];
    snprintf(path, sizeof(path), "%s/treasure.bin", dir_name);

    int fd = open(path, O_RDONLY);
    if(fd == -1){

        perror("Error while opening the treasure file\n");
        exit(1);
    }

    Treasure treasure;
    ssize_t bytesRead;
    printf("Content of %s:\n", path);
    printf("=====================================\n");

    while((bytesRead = read(fd, &treasure, sizeof(Treasure))) > 0){

        if(bytesRead != sizeof(Treasure)){

            printf("Error: Incomplete data read in treasure\n");
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

    if(bytesRead == -1){

        perror("Error while opening the treasure file\n");
    }

    close(fd);
}

//function to remove the Hunt directory along with the files containing in that directory
void remove_hunt(const char *dir_name){

    DIR *dir;
    struct dirent *entry;
    char path[MAX_LENGTH_NAME];

    dir = opendir(dir_name);
    if(dir == NULL){

        perror("opendir\n");
        exit(1);
    }

    while((entry = readdir(dir)) != NULL){

        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){

            continue;
        }

        snprintf(path, sizeof(path), "%s/%s\n", dir_name, entry->d_name);

        if(unlink(path) == -1){

            perror("unlink\n");
            closedir(dir);
            exit(1);
        }else{

            printf("Remove: %s\n", path);
        }
    }

    closedir(dir);

    if(rmdir(dir_name) == -1){

        perror("rmdir\n");
        exit(1);
    }else{

        printf("Hunt directory '%s' removed successfully\n", dir_name);
    }
}

// void remove_treasure(const char *dir_name, int treasureID){


// }

//main function in which we handle the commands in terminal
int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Error with the arguments\n");
        exit(1);
    }

    if (strcmp(argv[1], "--add") == 0) {
        if(argc < 3){

            printf("Error! Provide a directory name for --add\n");
            exit(1);
        }
        
        printf("Add\n");
        const char *dir_name = argv[2];
        createDirectory(dir_name);

        Treasure *treasure = addHunt_ID(1);
        strcpy(treasure->user_name, "Sara");
        treasure->gps.latitude = 45.1234;
        treasure->gps.longitude = 25.3333;
        strcpy(treasure->clue, "Behind the waterfall");
        treasure->value = 100;

        addTreasureToFile(dir_name, treasure);
        free(treasure);

    } else if (strcmp(argv[1], "--remove_hunt") == 0) {

        if(argc < 3){

            printf("Error! Provide a hunt directory name for --remove_hunt\n");
            exit(1);
        }

        printf("Remove Hunt\n");
        const char *dir_name = argv[2];
        remove_hunt(dir_name);

    }else if(strcmp(argv[1], "remove_treasure") == 0){

        printf("Remove treasure\n");

    } else if (strcmp(argv[1], "--list") == 0) {

        printf("Listing files in current directory:\n");
        listFilesInDirectory();

    } else if (strcmp(argv[1], "--view") == 0) {

        if(argc < 3){

            printf("Error! Provide a directory name for --view\n");
            exit(1);
        }

        printf("View\n");
        const char *dir_name = argv[2];
        viewTreasureInFile(dir_name);

    } else {

        printf("Please use a different argument. For example: --add, --list, --view, --remove\n");
    }

    return 0;
}