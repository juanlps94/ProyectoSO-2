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

void reservarMemoria(char ** cadena, int n){
    int it = 10*n - 10;     // Comezando desde el primer elemento del n-esimo bloque de 10
    while (it < 10*n)
    {
        cadena[it] = (char *) malloc (sizeof(char) * 1024);    // Reservamos memoria para cada string individual
        it++;
    }
}

// Función que ejecutaran los hilos "Trabajadores"
void * ordenamiento(void * argumentos){
    args infoArchivo = *(args *) argumentos;

    // Objetivo: Crear un array de strings que guarde las lineas del archivo

    typedef struct
    {
        char * cadena;
        int len;
    }Cadena;

    Cadena * cadena = malloc(sizeof (Cadena));
    //char ** lineas = malloc(sizeof (char *));
    //int lineas_len = 0;
    //int lineas_it = 0;
    int cadena_it = 0;
    int char_it = 0;
    FILE * fd;
    char ch;

    int max = 0;
    int min;
    
    fd = fopen( infoArchivo.nombre, "r");

    cadena[cadena_it].len = 0;              // cadena_it=0, inicializamos su len a 0
    cadena[cadena_it].cadena = realloc(cadena[cadena_it].cadena, sizeof(Cadena) * char_it+1);
    while ( (ch = fgetc(fd)) !=  EOF)
    {  
        cadena[cadena_it].len++;            // Aumentamos len de la cadena en 1
        if (ch == '\n')                     
        {
            cadena[cadena_it].cadena[char_it] = '\0';     // Terminamos la cadena en nulo
            if (cadena[cadena_it].len > 1){                 // Si la cadena es más larga que 1 (contando \n)
                cadena_it++;                                // Se cuenta la linea y se pasa a la siguiente
            }
            char_it = 0;                                    // Reinicilizamos el iterador de caracteres
            cadena[cadena_it].len = 0;                      // Inicializamos su len a 0
            cadena[cadena_it].cadena = realloc(cadena[cadena_it].cadena, sizeof(Cadena) * char_it+1); // Reservamos memoria para el primer caracter
            continue;
        }
        
        cadena[cadena_it].cadena[char_it] = ch;          // Guardamos el caracter en la linea
        char_it++;
        cadena[cadena_it].cadena = realloc(cadena[cadena_it].cadena, sizeof(Cadena) * char_it); // Reservamos memeria para un caracter más y aumentamos nos movemos a el
    }
/*
    if ( ch == EOF ){
            if (char_count > 1){ 
                line_count++;
                max = (char_count > max) ? char_count : max;
            }
    }
*/
    printf("%s\n", cadena[0].cadena);

    free(cadena);
    //free(lineas);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Comprobamos si el número de argumentos es correcto
    if (argc < 2){
        printf("Error: Muy pocos argumentos.\n");
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
        
