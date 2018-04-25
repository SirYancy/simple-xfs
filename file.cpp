#include "debug.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>

int GetFileList(char *buffer, char *ID) {
    struct dirent *entry;

    sprintf(buffer, "register;%s;", ID);
    // Open current directory
    char folder[25];
    strcpy(folder, ID);
    printf("folder %s\n", folder);
    DIR *dr = opendir(folder);

    if (dr == NULL) {
        printf("Could not open current directory\n" );
	std::cerr << strerror(errno) <<std::endl;
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
