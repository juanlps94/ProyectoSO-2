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

    char ** cadenas = (char **) malloc(sizeof (char *) * 10);     // Reservamos memoria para strings de 10 en 10
    char ** cadenas2;
    int count = 1;
    reservarMemoria(cadenas, count);
    
    char line[1024];    // Para guardar cada string

    FILE * archivo = fopen( infoArchivo.nombre, "r");

    fgets(line, 1024,archivo);
    if (line == NULL){
        printf("Archivo vacio.\n");
        pthread_exit(NULL);
    }

    int it = 0;
    do
    {   
        printf("%s\n", line);
        strcpy(cadenas[0], line);
        it++;
        if (it >= 10){
            it = 0;
            count++;
            cadenas2 = (char **) realloc(cadenas, count*10);
            cadenas = cadenas2; 
            reservarMemoria(cadenas, count);
        }
            
    } while ( fgets(line, 1024,archivo));
    
    free(cadenas);
    
    pthread_exit(NULL);
}


void * Comprobarlinea(char * nombre, int * arr){
    int cont=0;
    char caracter;  // Tamaño suficiente para la mayoría de las líneas
    int i=0;
    int* arrAux = arr;
    char ch;
    FILE *fp;
    fp = fopen(nombre, "r");        
        if (fp == NULL) {
            printf("Error al abrir el archivo: %s\n", nombre);
            exit(1);
        }

        while ((ch = fgetc(fp)) != EOF){
            
            if ((ch == '\n') || (ch == EOF)) {
                if(cont!=0){
                arrAux[i]=cont;
                cont=0;
                i++;
                }
            } else {
                    cont++;   
            }
        }
        arrAux[i]=cont;
       
  fclose(fp);
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
    
    int arr[1024]={}; 
    Comprobarlinea(archivo[0].nombre, arr);

    for (int i = 0; arr[i]!=0; i++){
        printf("Linea %d leida y son %d caracteres\n",i,arr[i]);
    }
    int max=arr[0];
    int min=arr[0];

    for (int i = 1; arr[i]!=0; i++){
        if(arr[i]>max) 
            max = arr[i];
        
        if(arr[i]<min) 
            min = arr[i];
    }

    printf("el valor max es %d\n",max);
    printf("el valor min es %d\n",min);

   
    /*char line[contador+1];    // Para guardar cada string
    FILE * arch = fopen( archivo[1].nombre, "r");

    fgets(line, contador+1,arch);
    if (line == NULL){
        printf("Archivo vacio.\n");
    }

        printf("%s\n", line);
        fclose(arch);
*/

    // Creamos n hilos trabajador

   /*
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
    */





	return 0;
}    
        
