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

    char  * a;
    reservar(a);
    strncpy(a, "Hola, Mundo.",10);
    printf("%s\n",a);



    return 0;
}