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

#define not !

// args guarda argumentos para los hilos
typedef struct
{
    char nombre[100];
    stats_t * stats;
}args;

typedef struct
{
    FILE * archivo1;
    FILE * archivo2;
    stats_t stats_f1;
    stats_t stats_f2;  
    FILE ** arreglo;
    stats_t * stats;
    int indice_nuevo_archivo;
}args_concatenacion;

typedef struct
    {
        char * cadena;
        int len;
}Cadena;

// Busca la cadena más larga y más corta y actualiza el stat del archivo según corresponda
void getMaxMin(Cadena * cadena, int cadena_it, args * info){
    info->stats->lineas_ordenadas = cadena_it;
    // Si el archivo esta vacio 
    if (cadena_it == 0) {
        strcpy(info->stats->linea_mas_corta, "");
        strcpy(info->stats->linea_mas_larga, "");
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
    info->stats->linea_mas_corta = malloc ( sizeof(char) * strlen(cadena[min].cadena));
    info->stats->linea_mas_larga = malloc ( sizeof(char) * strlen(cadena[max].cadena));
    // Actualizamos los stats correspondiente
    strcpy(info->stats->linea_mas_corta, cadena[min].cadena);
    strcpy(info->stats->linea_mas_larga, cadena[max].cadena);    
}

static int sort_lexicografica_decreciente(const void *p1, const void *p2){
    Cadena * cadena1 = (Cadena *) p1;
    Cadena * cadena2 = (Cadena *) p2;
    return strcasecmp( cadena2->cadena, cadena1->cadena );
}

// Función que ejecutaran los hilos "Trabajadores"
void * ordenamiento(void * argumentos){
    args * infoArchivo = (args *) argumentos;

    // Objetivo: Crear un array de strings que guarde las lineas del archivo

    Cadena * cadena = malloc(sizeof (Cadena));
    int cadena_it = 0;
    int char_it = 0;
    FILE * fd;
    char ch;

    int max;
    int min;
    
    fd = fopen( infoArchivo->nombre, "r");

    cadena[cadena_it].len = 0;              // cadena_it=0, inicializamos su len a 0
    cadena[cadena_it].cadena = malloc(sizeof(Cadena) * char_it+1);
    if (cadena[cadena_it].cadena == NULL) {    
        fprintf(stderr, "¡No se pudo reservar memoria!\n");
        exit(-1);
    }
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
            cadena[cadena_it].cadena = malloc( sizeof(char) ); // Reservamos memoria para el primer caracter
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

    getMaxMin(cadena, cadena_it, infoArchivo);

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

    strcat(infoArchivo->nombre, ".sorted");
    FILE * fd_out = fopen( infoArchivo->nombre, "w");

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
    printf("This worker thread writes %d lines to “%s”\n", cadena_it-descartadas, infoArchivo->nombre);
    // printf("Cadena más corta: \"%s\"\n", infoArchivo->stats.linea_mas_corta);
    // printf("Cadena más larga: \"%s\"\n\n", infoArchivo->stats.linea_mas_larga);

    // free(cadena);
    pthread_exit(NULL);
}

// Recibe un arreglo de "args" y los concatena los archivos que referencian en pares usando hilos que
// ejecutan "concatenacion_thread" generando un nuevo arreglo de "args" y repitiendo el proceso de forma

void * concatenacion_thread (void * argumentos){
    // file_1, file_2, stats_f1, stats_f2, arreglo, stats, indice
    args_concatenacion * info = (args_concatenacion *) argumentos; 

    int lineas_repetidas = 0;
    FILE * fp1 = info->archivo1;
    FILE * fp2 = info->archivo2;
    size_t linea_arch1_len = strlen(info->stats_f1.linea_mas_larga); // Largo maximo de linea para archivo 1
    size_t linea_arch2_len = strlen(info->stats_f2.linea_mas_larga); // Largo maximo de linea para archivo 2

    char ** linea_f1 = malloc( sizeof(char*) * info->stats_f1.lineas_ordenadas);
    for (size_t i = 0; i < info->stats_f1.lineas_ordenadas; i++) {
        linea_f1[i] = malloc ( sizeof(char) * linea_arch1_len);
    }
    
    char ** linea_f2 = malloc( sizeof(char*) * info->stats_f2.lineas_ordenadas);
    for (size_t i = 0; i < info->stats_f2.lineas_ordenadas; i++) {
        linea_f2[i] = malloc ( sizeof(char) * linea_arch2_len);
    }

    FILE * new_file = tmpfile();
    // Escribimos todo el archivo 1 en el nuevo archivo
    for (size_t i = 0; i < info->stats_f1.lineas_ordenadas; i++) {
        fgets(linea_f1[i], linea_arch1_len-1, fp1);
        fputs(linea_f1[i], new_file);
    }
    // Comparamos el archivo 2 con el archivo 1 en busca de lineas repetidas
    // e escribimos en el nuevo archivo las no repetidas.
    for (size_t i = 0; i < info->stats_f2.lineas_ordenadas; i++) {
        fgets(linea_f2[i], linea_arch1_len-1, fp2);
        for ( int j = 0; j < info->stats_f1.lineas_ordenadas; j++) {
            if( not strcmp(linea_f2[i], linea_f1[j])  ){
                fputs(linea_f2[i], new_file);
                continue;
            }
            lineas_repetidas++;
        }
    }

    // Guardamos el total de lineas despues de la fusión
    info->stats->lineas_ordenadas = linea_arch1_len + linea_arch2_len - lineas_repetidas;
    /*
    // Guardamos lineas más larga del nuevo archivo
    if ( linea_arch1_len > linea_arch2_len ) {
        strcpy( info->stats->linea_mas_larga , info->stats_f1.linea_mas_larga);
    }else{
        strcpy( info->stats->linea_mas_larga , info->stats_f2.linea_mas_larga);
    }
    // Guardamos linea más corta del nuevo archivo
    if ( strlen(info->stats_f1.linea_mas_corta) < strlen(info->stats_f2.linea_mas_corta)  ) {
        strcpy( info->stats->linea_mas_corta , info->stats_f1.linea_mas_corta);
    }else{
        strcpy( info->stats->linea_mas_corta , info->stats_f2.linea_mas_corta);
    }
    */    
    info->arreglo[info->indice_nuevo_archivo] = new_file;

};

// recursiva hasta tener un unico archivo referenciado
void concatenacion_recursiva(FILE ** files, stats_t * stats, int num_archivos){

    // Caso base
    if( num_archivos == 1 ){

        return;
    }

    int iteraciones = num_archivos / 2;     // Cuantas uniones se harán (por par)
    int cant_archivos_generados = iteraciones + (num_archivos % 2);    // Las uniones más el archivo sobrante, de haberlo.
    FILE * archivo_aux[cant_archivos_generados];
    stats_t arr_stats[cant_archivos_generados];
    // Indices
    int indice_archivo = 0;
    // Hilos 
    pthread_t concatenador[iteraciones];
    // Argumentos
    args_concatenacion info[iteraciones];
    
    for (size_t i = 0; i < iteraciones; i++)
    {
        // Llamada a concatenacion_thread con argumentos (archivo 1, archivo2 , archivo_aux, indice_nuevo_archivo)
        // Preparamos los argumetos que se le pasarán al hilo
        info[i].archivo1 = files[indice_archivo];
        info[i].archivo2 = files[indice_archivo + 1];
        info[i].stats_f1 = stats[indice_archivo];
        info[i].stats_f2 = stats[indice_archivo + 1];
        info[i].arreglo = archivo_aux;
        info[i].stats = arr_stats;
        info[i].indice_nuevo_archivo = i;
        
        pthread_create( &concatenador[i], NULL, concatenacion_thread, &info[i] );
        indice_archivo += 2;
    } 

    // Esperamos la terminacion de los hilo "concatenador"
    for (size_t i = 0; i < iteraciones; i++) {
        pthread_join( concatenador[i], NULL );
    }

    // Si la cantidad de archivos es impar, entonces queda uno sin concatenar. Lo añadimos al final del nuevo arreglo.
    if ( (num_archivos % 2) != 0 ){ 
        archivo_aux[cant_archivos_generados-1] = files[num_archivos-1];
    } 
 
    concatenacion_recursiva(archivo_aux, stats , cant_archivos_generados);

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
        archivo[i].stats = & stats[i];
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

    /*for(int i=0;i<cant_archivos;i++){
        strcat(archivo[i].nombre,".sorted");
    }*/

    FILE * aux_files[cant_archivos];
    for (size_t i = 0; i < cant_archivos; i++)
    {
        aux_files[i] = fopen( archivo[i].nombre, "r" );
        if (aux_files[i] == NULL){
            printf("Error al abrir archivo.\n");
        }
    }
    
    concatenacion_recursiva(aux_files, stats, cant_archivos);

	return 0;
}    