#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "client.h"
#include "pcb.h"
#include "states.h"
#include "operations.h"
#include "MMU.h"
#include <pthread.h>
#include <semaphore.h>
#define PORT "8006"

t_log* log_cpu;
char* logCpuString;

char* reasonForInterruption;

int memoryConnection; // Socket
int kernelSocket;
int cod_op_resource;
sem_t sExecutePcb;
sem_t sCanExecuteInstruction;
sem_t sInterrupt;
sem_t sExecuteCodeOpResources;
pthread_mutex_t sExecuteCodeOp;

t_config* iniciar_config(void);
void iterator(char* value);
t_package* package(int);
void server(char* port);
void serverInterrupt();
// void* runnerServerInterrupt(void* param);
// void* runnerServerDispatch(void* param);
void* runnerClientMemory(void* param);
int sendProgramCounter(int socketServer, int programCounter);
int conectMemoryProcess(int socketServer, int32_t pidPCBKernel);
void operate(void* clientFDPtr);

void askInstruction(int socketServer, instReqToMemory request);
uint32_t instReqToMemorySize(void);
t_config *config_CPU;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;
void executeInstrucions(void);
char* cpuClientValue;
char* msgMemory;
pcb* pcbExecute;
uint32_t pidExecute;
t_log* log_cpu;
clientConnection clientCpuDispatch; 
clientConnection clientCpuInterrupt;

char* logString; // Cada vez que se ejecute alguna de las siguientes funcines, lo que se guarde en el log, primero se guarda en esta variable y despues se libera
void logFetchInstruction(int pid, int programCounter);
void logExecutedInstruction(int pid, char* instruction, char* params);
void logTlbHit(int pid, int page);
void logTlbMiss(int pid, int page);
void logGetMacro(int pid, int page, int marco);
void logReadMemory(int pid, char* dir, char* value);
void logWriteMemory(int pid, char* dir, char* value);

int32_t pagesLength = -1;


//OJO QUE ACA ESTA DEFINIDO PERO TIENE QUE ESTAR EN CPU
t_list* TLB;