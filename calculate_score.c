#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_manager.h"  // Use the header where Treasure is defined

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_directory>\n", argv[0]);
        return 1;
    }

    char *dir = argv[1];
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/treasure.bin", dir);

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    Treasure t;
    int total_score = 0;

    while (fread(&t, sizeof(Treasure), 1, fp) == 1) {
        total_score += t.value;
    }

    fclose(fp);

    char msg[256];
    snprintf(msg, sizeof(msg), "Total score for hunt '%s': %d", dir, total_score);

    // Optional: log_op() if available, or just print
    printf("%s\n", msg);
    return 0;
}
