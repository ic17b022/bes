#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

void do_dir(const char *dir_name, const char *const *parms);

void do_file(const char *file_name, const char *const *parms);

int main(int argc, char *argv[]) {
    int iRc = EXIT_SUCCESS;

    if (argc > 1) {
        printf("Ivalid arguments\nCorrect usage: No arguments");
        iRc = EXIT_FAILURE;
    }

    do_file("/home/manuel/Documents/FH/Sem2", NULL);

    return iRc;
}

void do_dir(const char *dir_name, const char *const *parms) {
    DIR *directory = opendir(dir_name);
    struct dirent *entry;
    char *temp;

    while (entry = readdir(directory)) {
        if(strcmp(entry->d_name, "..") == 0 ||
                strcmp(entry->d_name, ".") == 0)
            continue;
        temp = malloc(strlen(dir_name) + strlen(entry->d_name) + 2);  //+1 for '/', +1 for '\n'
        if (temp) {
            temp[0] = '\0';

            strcat(temp, dir_name);
            strcat(temp, "/");
            strcat(temp, entry->d_name);

            do_file(temp, parms);

            free(temp);
        }
    }

    closedir(directory);
}

void do_file(const char *file_name, const char *const *parms) {
    struct stat buf;

    int retWert = stat(file_name, &buf);
    if (retWert == 0) {
        printf("%s\n", file_name);
        if (S_ISDIR(buf.st_mode)) {
            do_dir(file_name, parms);
        }
    } else {
        printf("%d", errno);
    }
}