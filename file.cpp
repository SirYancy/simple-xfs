#include "debug.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>

int GetFileList(char *buffer) {
    struct dirent *entry;

    // Open current directory
    DIR *dr = opendir(".");

    if (dr == NULL) {
        printf("Could not open current directory\n" );
        return 0;
    }

    while ((entry = readdir(dr)) != NULL) {
        if ((strcmp(".", entry->d_name) == 0) || (strcmp("..", entry->d_name) == 0)) {
            // Ignore these directories
            continue;
        } else {
            // Concat the name to buffer
            strcat(buffer, entry->d_name);
            strcat(buffer, ";");
        }
    }

    closedir(dr);
    return strlen(buffer);
}
