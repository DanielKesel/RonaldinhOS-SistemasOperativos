#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "processToMemory.h"


processToMemory* newProcessToMemory(uint32_t pid, const char* path) {
    // Reserva memoria para una nueva estructura processToMemory
    processToMemory* newProcess = malloc(sizeof(processToMemory));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newProcess == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newProcess->pid = pid;
    newProcess->tamanioProceso = 0;
    // Reserva memoria para el path y copia el path proporcionado
    newProcess->path = malloc(strlen(path) + 1); // +1 para el carácter nulo
    if (newProcess->path == NULL) {
        free(newProcess); // Libera la memoria asignada para el proceso si falla la asignación de path
        return NULL; // Error en la asignación de memoria para path
    }
    strcpy(newProcess->path, path);
    // Devuelve el puntero a la nueva estructura processToMemory
    return newProcess;
}

uint32_t processToMemoryStreamSize(processToMemory* process) {
    uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del pid
    size += sizeof(uint32_t); // Tamaño de tamanioProceso
    size += sizeof(uint32_t); // Tamaño de la longitud del path
    size += strlen(process->path) + 1; // Tamaño del path incluyendo el carácter nulo
    return size;
 
}

// Función para liberar la memoria del processToMemory
void freeProcessToMemory(processToMemory* process) {
    if (process != NULL) {
        free(process->path); // Libera la memoria asignada para el path
        free(process); // Libera la memoria asignada para la estructura
    }
}

