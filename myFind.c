//*
// @file myFind.c
// Betriebssysteme myFind Beispiel
// Beispiel 1
//
// @author Manuel Seifner
// @author Oliver Safar
// @date 2018/03/19
//
// @version 001
//
//

// -------------------------------------------------------------- includes --
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
#include <libgen.h>
#include <fnmatch.h>
#include <stdbool.h>
// --------------------------------------------------------------- defines --

// -------------------------------------------------------------- typedefs --

// --------------------------------------------------------------- globals --

// ------------------------------------------------------------- functions --

bool do_dir(const char* dir_name, const char* const* parms);

bool do_file(const char* file_path, const char* const* parms);

void getPermissionsString(__mode_t mode, char* permissions);

void getDateString(char* s, size_t size, time_t time);


int getDigitsCountFromInt(int i);

/**
 * \brief
 *
 *     Diese Funktion gibt den übergebenen Parameter
 *     auf der Konsole aus.
 *
 * \param	parameter  Auszugebender Parameter
 * \return	      Status-Code
 *
 */
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
        if(path[strlen(path)-1]=='/') {
                path[strlen(path)-1]='\0';
        }
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
                if (argv[index + 1] != NULL && (strcmp(argv[index + 1],"b")==0 ||
                                                strcmp(argv[index + 1],"c")==0 ||
                                                strcmp(argv[index + 1],"d")==0 ||
                                                strcmp(argv[index + 1],"p")==0 ||
                                                strcmp(argv[index + 1],"f")==0 ||
                                                strcmp(argv[index + 1],"l")==0 ||
                                                strcmp(argv[index + 1],"s")==0))
                {
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
    if (iRc == EXIT_SUCCESS) {
        char* tempPath = strdup(path);
        if (chdir(dirname(tempPath)) ==
            0) {//TODO: path darf kein trailing / haben. // Nachdem das script ohne / testet -> optional?
            if (!do_file(path, parms)) {
                iRc = EXIT_FAILURE;
            }
        }
        else {
            printf("myFind: '%s': %s \n", path, strerror(errno));
            iRc = EXIT_FAILURE;
        }

        free(tempPath);
    }

    if (path)
        free(path);

    return iRc;
}

/**
 * \brief Iterates over all Files in a directory and passes each file to the do-file function.
 *
 *     This function uses the opendir system call to open a directory and then readdir to pass each file
 *     into the do_file function.
 *
 *
 * \param	dir_name    directory to be opened by the function
 * \param   parms       parameter array defining which files should be printed as output
 *
 * \return	      void
 *
 */
bool do_dir(const char* dir_name, const char* const* parms) {
    bool rc = true;
    DIR* directory = NULL;
    struct dirent* entry;
    int arrSize = 10;
    int arrIndex = 0;
    char* tempPath = NULL;
    char* dirName = NULL;
    char** dir_array = malloc(sizeof(char*) * arrSize);  //TODO: and if not?

    tempPath = strdup(dir_name);
    dirName = basename(tempPath);
    directory = opendir(dirName);

    if (directory && chdir(dirName) == 0) {
        while ((entry = readdir(directory))) {
            if (strcmp(entry->d_name, "..") == 0 ||
                strcmp(entry->d_name, ".") == 0)
                continue;

            if (arrIndex == arrSize) {
                arrSize *= 2;
                dir_array = realloc(dir_array, sizeof(char*) * arrSize); //TODO: and if not?
            }


            dir_array[arrIndex] = malloc(strlen(dir_name) + strlen(entry->d_name) + 2);  // +1 for '/', +1 for '\0'
            if (dir_array[arrIndex]) {     //TODO: and if not?
                dir_array[arrIndex][0] = '\0';

                strcat(dir_array[arrIndex], dir_name);
                strcat(dir_array[arrIndex], "/");
                strcat(dir_array[arrIndex], entry->d_name);

                //do_file(temp, parms);
                arrIndex++;
            }
        }
        closedir(directory);
    } else {
        rc = false;
        printf("myFind: '%s': %s \n", dir_name, strerror(errno));
    }

    for (int i = 0; i < arrIndex && rc == true; i++) {
        rc = do_file(dir_array[i], parms);
        free(dir_array[i]);
    }

    free(tempPath);
    free(dir_array);

    if (chdir("..") != 0) {
        rc = false;
        printf("myFind: '%s': %s \n", dir_name, strerror(errno));
    }


    return rc;
}

/**
 * \brief Gets single file from do_dir and decides if file should be output or recursively calls do_dir in case
 *          the file is a directory.
 *
 *     This function uses the lstat system call do gather information about the file. The information is passed
 *     to the print function. If the file is a directory, do_dir is recursivly called.
 *
 *
 * \param	file_path   file to be checked by the function
 * \param   parms       parameter array defining which files should be printed as output
 *
 * \return	      void
 *
 */
bool do_file(const char* file_path, const char* const* parms) {
    bool rc = true;
    struct stat buf;
    int i = 0;
    char* tempPath = NULL;
    char* fileName = NULL;

    tempPath = strdup(file_path);
    fileName = basename(tempPath);
    if (lstat(fileName, &buf) == 0) {
        while (parms[i]) {
            if (strcmp(parms[i], "-print") == 0)
                printf("%s\n", file_path);
            else if (strcmp(parms[i], "-ls") == 0) {
                int posixly_correct_divisor = 2;
                char permissions[11];
                char dateString[13];
                struct group* grp;
                struct passwd* pwd;

                if (getenv("POSIXLY_CORRECT"))
                    posixly_correct_divisor = 1;

                getPermissionsString(buf.st_mode, permissions);
                getDateString(dateString, sizeof(dateString), buf.st_mtim.tv_sec);
                pwd = getpwuid(buf.st_uid);
                grp = getgrgid(buf.st_gid);

                if (pwd && grp)
                    printf("%9lu %7li %10s %3lu %8s %8s %10li %s %s\n", buf.st_ino,
                           buf.st_blocks / posixly_correct_divisor, permissions, (unsigned long) buf.st_nlink,
                           pwd->pw_name, grp->gr_name, buf.st_size, dateString,
                           file_path); //size of __nlink_t is plattform dependent. Cast to long int should be safe.
                else if (!pwd && !grp)
                    printf("%9lu %7li %10s %3lu %u %u %10li %s %s\n", buf.st_ino,
                           buf.st_blocks / posixly_correct_divisor, permissions, (unsigned long) buf.st_nlink,
                           buf.st_uid, buf.st_gid, buf.st_size, dateString,
                           file_path); //size of __nlink_t is plattform dependent. Cast to long int should be safe.
                else if (!pwd)
                    printf("%9lu %7li %10s %3lu %u %8s %10li %s %s\n", buf.st_ino,
                           buf.st_blocks / posixly_correct_divisor, permissions, (unsigned long) buf.st_nlink,
                           buf.st_uid, grp->gr_name, buf.st_size, dateString,
                           file_path); //size of __nlink_t is plattform dependent. Cast to long int should be safe.
                else
                    printf("%9lu %7li %10s %3lu %8s %u %10li %s %s\n", buf.st_ino,
                           buf.st_blocks / posixly_correct_divisor, permissions, (unsigned long) buf.st_nlink,
                           pwd->pw_name, buf.st_gid, buf.st_size, dateString,
                           file_path); //size of __nlink_t is plattform dependent. Cast to long int should be safe.

            } else if (strcmp(parms[i], "-user") == 0) {
                struct passwd* pwd = NULL;
                pwd = getpwnam(parms[i + 1]);

                if (pwd) {
                    if (pwd->pw_uid == buf.st_uid)
                        i++;
                    else
                        break;
                } else {
                    char* end = NULL;
                    pwd = getpwuid((uint) strtol(parms[i + 1], &end, 10));
                    if (*end == '\0' && pwd != NULL) {
                        if (pwd->pw_uid == buf.st_uid)
                            i++;
                        else
                            break;
                    } else {
                        rc = false;
                        fprintf(stderr, "myFind: '%s' is not the name of a known user \n", parms[i + 1]);
                        break;
                    }
                }
            } else if (strcmp(parms[i], "-type") == 0) {
                if ((strcmp("b", parms[i + 1]) == 0 && S_ISBLK(buf.st_mode)) ||
                    (strcmp("c", parms[i + 1]) == 0 && S_ISCHR(buf.st_mode)) ||
                    (strcmp("d", parms[i + 1]) == 0 && S_ISDIR(buf.st_mode)) ||
                    (strcmp("p", parms[i + 1]) == 0 && S_ISFIFO(buf.st_mode)) ||
                    (strcmp("f", parms[i + 1]) == 0 && S_ISREG(buf.st_mode)) ||
                    (strcmp("l", parms[i + 1]) == 0 && S_ISLNK(buf.st_mode)) ||
                    (strcmp("s", parms[i + 1]) == 0 && S_ISSOCK(buf.st_mode)))
                    i++;
                else {
                    break;
                }

            } else if (strcmp(parms[i], "-name") == 0) {
                if (fnmatch(parms[i + 1], fileName, FNM_NOESCAPE) == 0)
                    i++;
                else
                    break;
            }
            i++;
        }


        if (S_ISDIR(buf.st_mode) && rc == true) {
            rc = do_dir(file_path, parms);
        }
    } else {
        rc = EXIT_FAILURE;
        printf("myFind: %s \n", strerror(errno));
    }

    free(tempPath);

    return rc;
}
/**
 * \brief Checks permisson of a file and returns them in a permission string
 *
 *      File permissions are passed to the function, are checked and put into
 *
 *
 * \param	mode              mode?
 * \param   permissions       pointer to a permission string that is filled in the funciton
 *
 * \return	      void
 *
 */
void getPermissionsString(__mode_t mode, char* permissions) {
    strcpy(permissions, "----------");

    if (S_ISDIR(mode))
        permissions[0] = 'd';
    else if (S_ISLNK(mode))
        permissions[0] = 'l';
    else if (S_ISFIFO(mode))
        permissions[0] = 'p';
    else if (S_ISCHR(mode))
        permissions[0] = 'c';
    else if (S_ISBLK(mode))
        permissions[0] = 'b';

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

/* negative numbers get treated the same as positive ones. The sign is not counted */
int getDigitsCountFromInt(int i) {
    int n = 0;

    if (i == 0)
        return 1;

    while (i) {
        i /= 10;
        n++;
    }
    return n;
}
// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: