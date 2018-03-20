#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void do_dir(const char* dir_name, const char* const* parms);

void do_file(const char* file_name, const char* const* parms);

char* get_cwd(void);

void getPermissionsString(__mode_t mode, char* permissions);

void getDateString(char* s, size_t size, time_t time);

int main(int argc, char* argv[]) {
    int iRc = EXIT_SUCCESS;
    int is_output_set = 0;
    //Getting Path
    char* path = NULL;
    int index = 1;


    //Preparing the Parms Array
    const char* parms[argc - index];
    for (int i = 0; i <= argc - index; i++) {
        parms[i] = NULL;
    }

    if (argc < 2) {
        iRc = EXIT_FAILURE;
        printf("Ivalid arguments: myfind needs at least one Argument\nCorrect usage: myfind <file or directory> [ <aktion> ] ...\n");
    }
    if (iRc == EXIT_SUCCESS) {
        path = malloc(strlen(argv[index]) + 1);
        strcpy(path, argv[index]);
        index++;

        //index=2;
        //Check User Input
        while (index < argc) {
            if (strcmp(argv[index], "-user") == 0) {
                if (argv[index + 1] != NULL) {
                    parms[index - 2] = argv[index];
                    parms[index - 1] = argv[index + 1];
                    index = index + 2;
                    continue;
                } else {
                    printf("Ivalid arguments\nCorrect usage: -user <name>/<uid>\n");
                    iRc = EXIT_FAILURE;
                }
            }
            if (strcmp(argv[index], "-name") == 0) {
                if (argv[index + 1] != NULL) {
                    parms[index - 2] = argv[index];
                    parms[index - 1] = argv[index + 1];
                    index = index + 2;
                    continue;
                } else {
                    printf("Ivalid arguments\nCorrect usage: -name <pattern>\n");
                    iRc = EXIT_FAILURE;
                }
            }
            if (strcmp(argv[index], "-type") == 0) {
                if (argv[index + 1] != NULL) {
                    parms[index - 2] = argv[index];
                    parms[index - 1] = argv[index + 1];
                    index = index + 2;
                    continue;
                } else {
                    printf("Ivalid arguments\nCorrect usage: -type [bcdpfls]\n");
                    iRc = EXIT_FAILURE;
                }
            }
            if (strcmp(argv[index], "-print") == 0) {
                parms[index - 2] = argv[index];
                index++;
                is_output_set++;
                continue;
            }
            if (strcmp(argv[index], "-ls") == 0) {
                parms[index - 2] = argv[index];
                index++;
                is_output_set++;
                continue;
            }
            printf("Ivalid argument: %s \nSupported Arguments:\n-user <name>/<uid>\n-name <pattern>\n-type [bcdpfls]\n-print\n-ls",
                   argv[index]);
            iRc = EXIT_FAILURE;
            break;
        }
    }
    //add -print if no other output action was defined
    if (!is_output_set) parms[index - 2] = "-print";
    //call do_file
    if (iRc == EXIT_SUCCESS)
        do_file(path, parms);

    if (path)
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

    if (directory) {
        while ((entry = readdir(directory))) {
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
    } else {
        printf("myFind: '%s': %s \n", dir_name, strerror(errno));
    }
}

void do_file(const char* file_name, const char* const* parms) {
    struct stat buf;
    int i = 0;
    int posixly_correct_divisor = 2;
    char permissions[11];
    struct group* grp;
    struct passwd* pwd;
    char dateString[13];

    int retWert = lstat(file_name, &buf);
    if (retWert == 0) {
        while (parms[i]) {
            if (strcmp(parms[i], "-print") == 0)
                printf("%s\n", file_name);
            else if (strcmp(parms[i], "-ls") == 0) {
                if (getenv("POSIXLY_CORRECT"))
                    posixly_correct_divisor = 1;

                getPermissionsString(buf.st_mode, permissions);
                getDateString(dateString, sizeof(dateString), buf.st_mtim.tv_sec);
                grp = getgrgid(buf.st_gid);
                pwd = getpwuid(buf.st_uid);

                printf("%9lu %7li %10s %3lu %8s %8s %10li %s %s\n", buf.st_ino, buf.st_blocks / posixly_correct_divisor,
                       permissions, buf.st_nlink, pwd->pw_name, grp->gr_name, buf.st_size, dateString, file_name);
            } else if (strcmp(parms[i], "-name") == 0) {
                if (strcmp(getpwuid(buf.st_uid)->pw_name, parms[i + 1]) != 0)
                    break;
                i++;
            } else if (strcmp(parms[i], "-type") == 0) {
                if ((strcmp("b", parms[i + 1]) == 0 && S_ISBLK(buf.st_mode)) ||
                    (strcmp("c", parms[i + 1]) == 0 && S_ISCHR(buf.st_mode)) ||
                    (strcmp("d", parms[i + 1]) == 0 && S_ISDIR(buf.st_mode)) ||
                    (strcmp("p", parms[i + 1]) == 0 && S_ISFIFO(buf.st_mode)) ||
                    (strcmp("f", parms[i + 1]) == 0 && S_ISREG(buf.st_mode)) ||
                    (strcmp("l", parms[i + 1]) == 0 && S_ISLNK(buf.st_mode)) ||
                    (strcmp("s", parms[i + 1]) == 0 && S_ISSOCK(buf.st_mode))) {
                    i++;
                    continue;
                } else
                    break;
            }
            i++;
        }


        if (S_ISDIR(buf.st_mode)) {
            do_dir(file_name, parms);
        }
    } else {
        printf("%s \n", strerror(errno));
    }
}

void getPermissionsString(__mode_t mode, char* permissions) {
    strcpy(permissions, "----------");

    if (S_ISDIR(mode))
        permissions[0] = 'd';
    else if (S_ISLNK(mode))
        permissions[0] = 'l';

    if (S_IRUSR & mode)
        permissions[1] = 'r';
    if (S_IWUSR & mode)
        permissions[2] = 'w';
    if (S_IXUSR & mode)
        permissions[3] = 'x';
    if (S_IRGRP & mode)
        permissions[4] = 'r';
    if (S_IWGRP & mode)
        permissions[5] = 'w';
    if (S_IXGRP & mode)
        permissions[6] = 'x';
    if (S_IROTH & mode)
        permissions[7] = 'r';
    if (S_IWOTH & mode)
        permissions[8] = 'w';
    if (S_IXOTH & mode)
        permissions[9] = 'x';
}

void getDateString(char* s, size_t size, time_t time) {
    strftime(s, size, "%b %e %H:%M", localtime(&time));
}