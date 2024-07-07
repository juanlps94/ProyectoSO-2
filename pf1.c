//
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
// incluye la cabecera
#include "pf1.h"

// args guarda argumentos para los hilos
typedef struct
{
    char nombre[100];
    stats_t stats;
}args;

// Función que ejecutaran los hilos "Trabajadores"
void * ordenamiento(){

}

int main(int argc, char *argv[])
{
    // Comprobamos si el número de argumentos es correcto
    if (argc < 3){
        printf("Error: Muy pocos arguentos.\n");
        exit(0);
        //error(0); // ?
    }
    stats_t stats[argc - 1]; // Por defecto

    int cant_archivos =  argc - 1;
    args archivo[cant_archivos];
    
    // Guardamos el nombre de cada archivo en su correspondiente estrucuta
    for (size_t i = 0; i < cant_archivos; i++)
    {
        strcpy(archivo[i].nombre, argv[i+1]); 
    }
    
    // Creamos n hilos trabajador
    pthread_t trabajador[cant_archivos];
    for (size_t i = 0; i < cant_archivos; i++)
    {
        pthread_create(&trabajador[i], NULL, ordenamiento, &archivo[i]);
    }

    // Esperamos la terminación de todos los hilos
    for (size_t i = 0; i < cant_archivos; i++)
    {
        pthread_join(trabajador[i], NULL);
    }

	return 0;
}    
        
