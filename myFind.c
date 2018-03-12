#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

void do_dir(const char* dir_name, const char* const* parms);

void do_file(const char* file_name, const char* const* parms);

char* get_cwd(void);

int main(int argc, char* argv[]) {
    int iRc = EXIT_SUCCESS;

    //Preparing the Parms Array
    const char* parms[5] = {'\0'};

    //Getting Path
    char* path;
    int index = 1;
    if (argv[index] != NULL && *argv[index] != '-') {
        path = malloc(strlen(argv[index]) + 1);
        strcpy(path, argv[index]);
        index++;
    } else {
        path = get_cwd();
    }
    //Check User Input
    while (index < argc) {
        if (strcmp(argv[index], "-user") == 0) {
            if (argv[index + 1] != NULL && *argv[index + 1] != '-') {
                parms[0] = argv[index + 1];
            } else {
                printf("Ivalid arguments\nCorrect usage: -user <name>/<uid>\n");
                iRc = EXIT_FAILURE;
            }
        }
        if (strcmp(argv[index], "-name") == 0) {
            if (argv[index + 1] != NULL && *argv[index + 1] != '-') {
                parms[1] = argv[index + 1];
            } else {
                printf("Ivalid arguments\nCorrect usage: -name <pattern>\n");
                iRc = EXIT_FAILURE;
            }
        }
        if (strcmp(argv[index], "-type") == 0) {
            if (argv[index + 1] != NULL && *argv[index + 1] != '-') {
                parms[2] = argv[index + 1];
            } else {
                printf("Ivalid arguments\nCorrect usage: -type [bcdpfls]\n");
                iRc = EXIT_FAILURE;
            }
        }
        if (strcmp(argv[index], "-print") == 0) {
            parms[3] = argv[index];
        }
        if (strcmp(argv[index], "-ls") == 0) {
            parms[4] = argv[index];
        }

        index++;
    }
    //add -print if no other output action was defined
    if (parms[3] == NULL && parms[4] == NULL) {
        parms[3] = "-print";
    }
    //call do_file
    if (iRc == EXIT_SUCCESS)
        do_file(path, parms);

    free(path);
    return iRc;
}

//get current working directory
char* get_cwd(void) {
    unsigned int size = 100;

    while (1) {
        char* buffer = (char*) malloc(size);
        if (getcwd(buffer, size) == buffer)
            return buffer;
        free(buffer);
        //if (errno != ERANGE)
        //return 0;
        size *= 2;
    }
}

void do_dir(const char* dir_name, const char* const* parms) {
    DIR* directory = opendir(dir_name);
    struct dirent* entry;
    char* temp;

    if(directory) {
        while ((entry = readdir(directory))) {      //yes, assignment is intended here.
            if (strcmp(entry->d_name, "..") == 0 ||
                strcmp(entry->d_name, ".") == 0)
                continue;
            temp = malloc(strlen(dir_name) + strlen(entry->d_name) + 2);  // +1 for '/', +1 for '\n'
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
    }else{
        printf("myFind: '%s': %s \n", dir_name, strerror(errno));
    }
}

void do_file(const char* file_name, const char* const* parms) {
    struct stat buf;

    int retWert = lstat(file_name, &buf);
    if (retWert == 0) {
        if (parms[3])
            printf("%s\n", file_name);

        if (S_ISDIR(buf.st_mode)) {
            do_dir(file_name, parms);
        }
    } else {
        printf("%s \n", strerror(errno));
    }
}