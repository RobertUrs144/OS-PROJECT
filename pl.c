#include #include #include #include #include #include
#define MAX_LENGTH_NAME 100

typedef struct{

int treasureID;
char user_name[MAX_LENGTH_NAME];
char clue[100];
char gps_coordinates[100];
int value;
}Treasure;

Treasure *addHunt_ID(int treasureID){

Treasure *temp = (Treasure*)malloc(sizeof(Treasure));
if(!temp){

printf("Memory allocation failed\n");
exit(1);
}

temp->treasureID = treasureID;

return temp;
}

int main(int argc, char **argv){

if(argc < 1){

printf("Error with the arguments\n");
exit(1);
}

if(strcmp(argv[1], "--add") == 0){

printf("Add\n");
return 0;

}else if(strcmp(argv[1], "--remove") == 0){

printf("Remove\n");
return 0;

}else if(strcmp(argv[1], "--list") == 0){

printf("List data\n");
return 0;

}else if(strcmp(argv[1], "--view") == 0){

printf("View\n");
return 0;

}else{

printf("Please use a different argument. For example: add, list, view, remove");
return 0;
}

DIR *dir;
struct dirent *entry;

dir = opendir(".");

if(dir == NULL){

perror("opendir");
return 1;
}

while((entry = readdir(dir)) != NULL){

printf("%s\n", entry->d_name);
}

closedir(dir);

return 0;
}

