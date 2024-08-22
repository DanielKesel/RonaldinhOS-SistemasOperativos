#ifndef PROCESSTOMEMORY_H
#define PROCESSTOMEMORY_H
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    uint32_t pid;
    char*    path;
    uint32_t tamanioProceso;
} processToMemory;

typedef struct
{
    uint32_t pid;
    
} cpuPidAndCod;


processToMemory* newProcessToMemory(uint32_t pid, const char* path);
uint32_t processToMemoryStreamSize(processToMemory* process);
void freeProcessToMemory(processToMemory* process);
processToMemory* recvProcessToMemory(int socketClient);


#endif /* PROCESSTOMEMORY */