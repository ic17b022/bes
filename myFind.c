#include <stdio.h>

int main(int argc, char* argv[]) {
    int iRc = 0;

    if(argc > 0)
    {
        printf("Ivalid arguments\nCorrect usage: No arguments");
        iRc = 1;
    }

    return iRc;
}