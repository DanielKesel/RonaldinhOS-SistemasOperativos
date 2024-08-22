// EJECUTAR_SCRIPT scripts_kernel/PRUEBA_IO
// INICIAR_PLANIFICACION
#include <commons/collections/queue.h>
//#include <commons/collections/list.h>
#include <semaphore.h>
#include <stdbool.h>
#include "client_k.h"
#include "server_k.h"
#include "pcb.h"
#include "states.h"
#include "interactive_console.h"
#include "IOFUNCTIONS.h"
#define MAX_INTERFACES 6
typedef enum {
	CONN_CPU_DISPATCH,
	CONN_CPU_INTERRUPT,
	CONN_MEMORY
} connections;

typedef struct {
	uint32_t pid;
    char* operation;
} ioParams;


planification chosenPlanner;
uint32_t quantum;

void asignPlanner(char* plan);

t_config* iniciar_config(void);
void iterator(char* value);

// Mensajes para identificar hilo
char* msgMemory;
char* msgCpuDispatch;
char* msgCpuInterrupt;

// Todo lo que sale del archivo config
char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMO_PLANIFICACION;
char* QUANTUM;
char* RECURSOS;
char* INSTANCIAS_RECURSOS;
char* GRADO_MULTIPROGRAMACION;
char* PATH_PRUEBAS;
char** recursosArray;
size_t recursosSize;

Recurso* recursos;

t_log* log;
void createLog(void);

int socketCpuDispatch;
int socketCpuInterrupt;
int socketMemory;

// Los procesos que ejecutan cada hilo
void* runnerServer(void* param);
void* runnerOperate(void* param);
//void* runnerClientMemory(void* param);
//void* runnerClientCpuDispatch(void* param);
//void* runnerClientCpuInterrupt(void* param);
void* runnerProcessNewToReady(void* param); // Se mueven pcbs nuevos a ready mientras el grado de multiprogramacion lo permita
//void* runnerProcessToExit(void* param); // Se destruyen los pcbs que finalizan
void* runnerProcessToExecute(void* param); //
//void* runnerConsole(void* param);

void server(void); // Se encarga solo de recibir, las peticiones que le hagan a Kernel va a la cola queueServer
void operate(void* clientFDPtr); // Se encarga de operar segun lo que se haya recibido dentro de la cola queueServer
//void client(char*, char*, char*);
void extractAllDataConfig(t_config*);
void console(void);
// void interpetAndExecCommand(t_queue* wordList);
// void printParameterScopeError();
// char* toUpperCase(char* str); // Devuelve otro char con todas las letras em mayus, y libera de la memoria el char que pasamos por parametro

void recivePcb(void);
void checkResponseOfCpu(void);
void ioFunction(void* params);

pcb* fifo();
void executePcb(pcb* pcbToExecute);
pcb* currentyRunningPcb;

pthread_mutex_t sQueueNews; // Semaforo para Sincronización Mutex
sem_t sQueueNewsAvaiable; // Cantidad de procesos que hay dentro de Ready

pthread_mutex_t sQueueReadys; // Semaforo para Sincronización Mutex
pthread_mutex_t sQueueAux;
sem_t sQueueReadysMax; // La cantidad maxima de procesos que puede entrar a Ready (Multiprogramacion)
sem_t sQueueReadysAvaiable; // Cantidad de procesos que hay dentro de Ready
sem_t sQueueAuxAvaiable; // Cantidad de procesos con prioridad por no terminar QUANTUM

sem_t sRunningProcess; // Binario que indica si hay algun proceso en ejecucion

sem_t sRunQuantum; // Semaforo para ejecutar el contador de Quantum

sem_t sCanPlan; // Semaforo que indica si puedo ejecutar el planificador
int plannerIsRunning;

pthread_mutex_t sListBlockeds; // Semaforo para Sincronización Mutex

pthread_mutex_t sListPcbToMemory;

// pthread_mutex_init(semaphore, NULL); // PARA INICIALIZAR
// pthread_mutex_destroy(semaphore); // PARA DESTRUIR
// pthread_mutex_lock(semaphore); // wait()
// pthread_mutex_unlock(semaphore); // signal()

t_queue* queueServer; // Cola de peticiones a Kernel
t_queue* queueNews; // Cola de nuevos procesos
t_queue* queueReadys; // Cola de procesos Listos
t_queue* queueAux; 	// Cola de procesos con prioridad
t_list* listBlockeds; // Cola de procesos Bloqueados
t_list*	listResourcesPcbs;
t_list* listPcbToMemory; // Lista de pid y path que van a memoria
bool findPcbToMemoryByPid(uint32_t pid, void* ptm);
bool findPcbByPid(uint32_t pid, void* voidPcb);

pcb* createPcb(char* path);
void deletePcb(pcb* pcbToDelete);

t_list* freePids;
int nextPid;
int getNewPid();
void releasePid(int pid);

void interruptProcessCpu(char* reason);

pcb* pcbToExecute;

void blockPcb(pcb* pcbToBlock);
pcb* desblockPcb(bool (*pcbSearchCriteria)(void*));

processToMemory* createProcessToMemory(uint32_t pid, char* path);
char* logString; // Cada vez que se ejecute alguna de las siguientes funcines, lo que se guarde en el log, primero se guarda en esta variable y despues se libera
char* stateToChar(pcb *pcb);
void logNewProcess(int pid);
void logExitProcess(int pid, char* reason);
void logChangeStateProcess(int pid, char* previousState, char* newState);
void logBlockingReasonProcess(int pid, char* bloquedFor);
void logFinishedQuantumProcess(int pid);
void logReadyPrioEntryProcess();
void logReadyEntryProcess();
void putPids(); // Esta funcion es para poner concatenar cada uno de los pids de la lista Ready (O cualquiera sea) en logString, esto pasa al ejecutar logReadyEntryProcess por ejemplo

int nextFreeIndex;
io* isInterfaceConnected(char* interfaceName);
io* interfacesIoActivas[MAX_INTERFACES];
io* checkIfIos(int socketCliente);

void queue_iterate(t_queue* queue, void (*func)(void*));

void stopPlanner();
void runPlanner();
void runChangeMultiprogramming(char* value);
void changeMultiprogramming(char* value);
void allPcbStates();
void finishProcess(uint32_t pid);
Recursos_de_PID* buscarRecursosDePID(t_list* listResourcesPcbs, int pid);
void liberarRecursos(uint32_t pid);