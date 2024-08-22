#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/hello.h>
#include <commons/log.h>
#include "server.h"
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "utils.h"
#include "utils_memory.h"
#include "serialization.h"
#include <pthread.h>
#define MAX_INSTRUCTIONS 100
#include <semaphore.h>
#include "paginacion.h"
#include "IOFUNCTIONS.h"
#include "utils_memory.h"

t_log* log_memory;
char* logMemoryString; // Cada vez que se ejecute alguna de las siguientes funcines, lo que se guarde en el log, primero se guarda en esta variable y despues se libera

pthread_mutex_t sQueueNews; // Semaforo para Sincronización Mutex
sem_t sCanUploadNewInstruction;
sem_t sCanGiveInstruction;
int instructionsUploaded;

// pthread_mutex_t sQueueServer; // Semaforo para Sincronización Mutex
// sem_t sQueueServerAvaiable; // Cantidad de peticiones que hay dentro de queueServer
char** instructions = NULL;
t_config* memoryConfig ;
t_log* memoryLogger;
int kernelClientFD; // Utiliza clientConnection definido en server.h
int numLineas;
char* memoryPort;
char* lengthMemory; //TAM_MEMORIA
char* lengthPage; //TAM_PAG
char* pathInstructions;
char* answerDelay;
int cod_op;
int memoryFD;
int pidPCBKernel; //hardcodeado para pruebas
char* pathOfPseudoCode; //Hardcodeado a espera de desarrollo del Kernel
t_queue* queueServer;
int cpuClientFD;

void iterator(char* value);

void server(void);
void operate(void* clientFDPtr);
char* getInstruction(instReqToMemory req);
void sendToCPU(clientConnection xClientFD);

int memLengthInt;
int pageLengthInt;

int cantFrames;
void* mainMemory;
char* dataForBitArray;
t_bitarray* bitArrayOcupiedFrames;
t_list* pageTable;
bool findProcessToMemoryByPid(uint32_t pid, void* ptm);
processToMemory* newProcess;

t_list* listProcessToMemory;

int idGlobal;

double tiempo;

pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexIdGlobal;