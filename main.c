#include <stdio.h>
#include <stdlib.h>

int main(int Argc, char** Argv) {

    if(Argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", Argv[0]);
    }

    char* P = Argv[1];

    printf("  .global main\n");
    printf("main:\n");

    //将第一个num传入a0
    printf("li a0, %ld\n", strtol(P, &P, 10));

    while(*P)
    {
        if(*P == '+')
        {
            ++P;
            printf("addi a0, a0, %ld\n", strtol(P, &P, 10));
        }
        else if(*P == '-')
        {
            ++P;
            printf("addi a0, a0, -%ld\n", strtol(P, &P, 10));
        }
    }

    printf("  ret\n");

    return 0;
}