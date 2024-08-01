//
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

void reservar(char * mem){

    mem = malloc(sizeof(char) * 10);
}

int main (){
    char * s[100];
    FILE * fd =  tmpfile();
    fprintf(fd, "Hola, mundo.\n");
    rewind(fd);
    fgetc(fd);
    printf("%c\n", s);
    return 0;
}