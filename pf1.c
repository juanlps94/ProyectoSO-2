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

typedef struct
{
    args archivo1;
    args archivo2;
    args * arreglo;
    int indice_nuevo_archivo;
}args_concatenacion;

typedef struct
    {
        char * cadena;
        int len;
}Cadena;

// Busca la cadena más larga y más corta y actualiza el stat del archivo según corresponda
void getMaxMin(Cadena * cadena, int cadena_it, args * info){
    // Si el archivo esta vacio 
    if (cadena_it == 0) {
        info->stats.lineas_ordenadas = cadena_it;
        strcpy(info->stats.linea_mas_corta, "");
        strcpy(info->stats.linea_mas_larga, "");
        return ;
    }
    // Hacemos la comparación usando los indices, asumimos que cadena[0].cadena es la cadena
    // más larga y la más corta, comparamos con el resto de cadenas y vamos actualizando max y min según
    // corresponda
    int max = 0;
    int min = 0;
    for (size_t i = 1; i < cadena_it; i++) {
        if (strlen(cadena[i].cadena) > strlen(cadena[max].cadena)  )
            max = i;
        if (strlen(cadena[i].cadena) < strlen(cadena[min].cadena)  )
            min = i;
    }

    // Reserva de memoria para almacenar las lineas más larga y más corta
    info->stats.linea_mas_corta = malloc ( sizeof(char) * strlen(cadena[min].cadena));
    info->stats.linea_mas_larga = malloc ( sizeof(char) * strlen(cadena[max].cadena));
    // Actualizamos los stats correspondiente
    strcpy(info->stats.linea_mas_corta, cadena[min].cadena);
    strcpy(info->stats.linea_mas_larga, cadena[max].cadena);    
}

static int sort_lexicografica_decreciente(const void *p1, const void *p2){
    Cadena * cadena1 = (Cadena *) p1;
    Cadena * cadena2 = (Cadena *) p2;
    return strcasecmp( cadena2->cadena, cadena1->cadena );
}

// Función que ejecutaran los hilos "Trabajadores"
void * ordenamiento(void * argumentos){
    args infoArchivo = *(args *) argumentos;

    // Objetivo: Crear un array de strings que guarde las lineas del archivo

    Cadena * cadena = malloc(sizeof (Cadena));
    int cadena_it = 0;
    int char_it = 0;
    FILE * fd;
    char ch;

    int max;
    int min;
    
    fd = fopen( infoArchivo.nombre, "r");

    cadena[cadena_it].len = 0;              // cadena_it=0, inicializamos su len a 0
    cadena[cadena_it].cadena = realloc(cadena[cadena_it].cadena, sizeof(Cadena) * char_it+1);
    while ( (ch = fgetc(fd)) !=  EOF)
    {  
        cadena[cadena_it].len++;            // Aumentamos len de la cadena en 1
        if (ch == '\n')                     
        {
            cadena[cadena_it].cadena[char_it] = '\0';       // Terminamos la cadena en nulo
            if (cadena[cadena_it].len > 1){                 // Si la cadena es más larga que 1 (contando \n)
                cadena_it++;                                // Se cuenta la linea y se pasa a la siguiente
                cadena = realloc(cadena, sizeof(Cadena) * (cadena_it+1));
            }
            char_it = 0;                                    // Reinicilizamos el iterador de caracteres
            cadena[cadena_it].len = 0;                      // Inicializamos su len a 0
            cadena[cadena_it].cadena = realloc(cadena[cadena_it].cadena, sizeof(char) * (char_it+1)); // Reservamos memoria para el primer caracter
            continue;
        }
        
        cadena[cadena_it].cadena[char_it] = ch;          // Guardamos el caracter en la linea
        char_it++;
        cadena[cadena_it].cadena = realloc(cadena[cadena_it].cadena, sizeof(char) * (char_it+1)); // Reservamos memoria para el primer caracter
    }

    if (cadena[cadena_it].len > 1){                 // Si la cadena es más larga que 1 (contando \n)
            cadena_it++;                            // Se cuenta la linea y se pasa a la siguiente
            cadena = realloc(cadena, sizeof(Cadena) * (cadena_it+1));
    }

    getMaxMin(cadena, cadena_it, &infoArchivo);

    printf("Cadena más corta: \"%s\"\n", infoArchivo.stats.linea_mas_corta);
    printf("Cadena más larga: \"%s\"\n\n", infoArchivo.stats.linea_mas_larga);

/*
    printf("Antes de ordenar\n");
    for (size_t i = 0; i < cadena_it; i++)
    {
        printf("%s\n", cadena[i].cadena);
    }
*/    
    qsort(cadena, cadena_it, sizeof(Cadena), sort_lexicografica_decreciente);
/*
    printf("\nDespués de ordenar\n");
    for (size_t i = 0; i < cadena_it; i++)
    {
        printf("%s\n", cadena[i].cadena);
    }
*/

    strcat(infoArchivo.nombre, ".sorted");
    FILE * fd_out = fopen( infoArchivo.nombre, "w");

    int descartadas = 0; // Cuentas las lineas descartadas por repeticion
    for (size_t i = 0; i < cadena_it; i++)
    {
        if ( i > 0 && ! (strcasecmp( cadena[i].cadena, cadena[i-1].cadena )) ){
            descartadas++;
            continue;
        }
        fputs(cadena[i].cadena, fd_out);
        if ( i != cadena_it-1 )
            fputs("\n", fd_out); 
        
    }
     
    // "Hola Mundo" y "Mundo Mundo " son consideradas distintas debido al espacio del final
    printf("This worker thread writes %d lines to “%s”\n", cadena_it-descartadas, infoArchivo.nombre);

    free(cadena);
    //free(lineas);

    pthread_exit(NULL);
}

// Función que ejecutaran los hilos "concatenador"
void * concatenacion_thread( void * argumentos){
    args_concatenacion info_concatenacion = *(args_concatenacion *) argumentos;

    pthread_exit(NULL);
};

// Recibe un arreglo de "args" y los concatena los archivos que referencian en pares usando hilos que
// ejecutan "concatenacion_thread" generando un nuevo arreglo de "args" y repitiendo el proceso de forma
// recursiva hasta tener un unico archivo referenciado
void concatenacion_recursiva(args * archivos, int num_archivos){

    // Caso base
    if( num_archivos == 1 )
        return;

    int iteraciones = num_archivos / 2; 
    int cant_archivos_generados = iteraciones + (num_archivos % 2);
    args archivo_aux[cant_archivos_generados];
    // Indices
    int indice_archivo = 0;
    //int indice_archivo_aux = 0;
    
    // Hilos 
    pthread_t concatenador[iteraciones];

    args_concatenacion info[iteraciones];
    
    for (size_t i = 0; i < iteraciones; i++)
    {
        // Llamada a concatenacion_thread con argumentos (archivo 1, archivo2 , archivo_aux, indice_nuevo_archivo)
        // Preparamos los argumetos que se le pasarán al hilo
        info[i].archivo1 = archivos[indice_archivo];
        info[i].archivo2 = archivos[indice_archivo + 1];
        info[i].arreglo = archivo_aux;
        info[i].indice_nuevo_archivo = i;
        pthread_create( concatenador[i], NULL, concatenacion_thread, &info[i] );
        indice_archivo += 2;
    } 

    // Esperamos la terminacion de los hilo "concatenador"
    for (size_t i = 0; i < iteraciones; i++) {
        pthread_join( concatenador[i], NULL );
    }

    // Si la cantidad de archivos es impar, entonces queda uno sin concatenar. Lo añadimos al final del nuevo arreglo.
    if ( (num_archivos % 2) != 0 ){ 
        archivo_aux[cant_archivos_generados-1] = archivos[num_archivos-1];
    } 

    concatenacion_recursiva(archivo_aux, cant_archivos_generados);

}

int main(int argc, char *argv[])
{
    // Comprobamos si el número de argumentos es correcto
    if (argc < 2) {  // Cambiar a 3
        printf("Error: Muy pocos argumentos.\n");
        exit(0);
        //error(0); // ?
    }
    stats_t stats[argc - 1]; // Por defecto

    int cant_archivos =  argc - 1;
    args archivo[cant_archivos];
    
    // Guardamos el nombre de cada archivo en su correspondiente estructura
    for (size_t i = 0; i < cant_archivos; i++) {
        strcpy(archivo[i].nombre, argv[i+1]); 
    }
    // Creamos n hilos trabajador
    pthread_t trabajador[cant_archivos];
    for (size_t i = 0; i < cant_archivos; i++) { 
        pthread_create(&trabajador[i], NULL, ordenamiento, &archivo[i]);
    }

    // Esperamos la terminación de todos los hilos trabajador
    for (size_t i = 0; i < cant_archivos; i++) {
        pthread_join(trabajador[i], NULL);
    }

    concatenacion_recursiva(archivo, cant_archivos);

	return 0;
}    


