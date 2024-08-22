#ifndef UTILS_H
#define UTILS_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/bitarray.h>
#include<string.h>
#include<assert.h>
#include <time.h>
#include "pcb.h"
#include "serialization.h"
#include "processToMemory.h"

// typedef enum {
// 	MESSAGE,
// 	PACKAGE,
// 	PCB
// }op_code;

typedef enum {
    KERNEL,
    CPU,
    MEMORY,
    IO,
} MODULE;

void printBuffer(void* buffer, uint32_t size);

extern const char* module_strings[];
MODULE detectModuleByName(const char* module_name);
typedef struct {
    uint32_t pid;
	uint32_t programCounter;
} instReqToMemory;

typedef struct {
    uint32_t pid;
	uint32_t timeToSleep;
} IO_GEN_SLEEP_params;


typedef struct {
    uint32_t pid;
    uint32_t physicalAdress;
	uint32_t lengthToReadOrWrite;
} IO_STDIN_READ;

typedef struct {
    uint32_t pid;
    uint32_t physicalAdress;
	char* textToMemory;
} IO_STDIN_READ_WRITE_MEMORY;

typedef struct {
    uint32_t pid;
    uint32_t physicalAdress;
	uint32_t lengthToReadOrWrite;
} IO_STDOUT_WRITE;

typedef struct {
    uint32_t pid;
	char* fileName;
} IO_FS_CREATE_params;
typedef struct {
    uint32_t pid;
	char* fileName;
} IO_FS_DELETE_params;

typedef struct {
    uint32_t pid;
	char* fileName;
    uint32_t length;
} IO_FS_TRUNCATE_params;

typedef struct {
    uint32_t pid;
    char* fileName;
    uint32_t physicalAdress;
	uint32_t length;
    uint32_t filePointer;
} IO_FS_WRITE_params;

typedef struct {
    uint32_t pid;
    char* fileName;
    uint32_t physicalAdress;
	uint32_t length;
    uint32_t filePointer;
} IO_FS_READ_params;

typedef struct {
    char* word;
    uint32_t pid;
    uint32_t length;
    uint32_t physicalAdress;
} IO_FS_READ_TO_MEMORY_params;

typedef struct {
    uint32_t pid;
    int length;
    uint32_t physicalAdress;
} MOVE_IN_TO_MEMORY_params;

typedef struct {
    uint32_t pid;
    int length;
    uint32_t physicalAdress;
    uint32_t dataRegistro
} MOVE_OUT_TO_MEMORY_params;

typedef struct {
    uint32_t pid;
    int paginas
} solicitudFrameMemoria;

typedef struct 
{
    int pid;
    int tamanioProceso;

} RESIZE_params;


typedef struct {
    char* recurso;
    int instancia;
} Recurso;

typedef struct {
    int pid;
    t_list* recursos;
} Recursos_de_PID;

typedef struct 
{
    int pid;
    int page;
    int frame;
    long lastUsedTime;

} TLB_ROW;


typedef struct {
    int pid;
    uint32_t dfDest;
    uint32_t dfOrig;
    uint32_t tam;
} COPY_STRING_params;



void sendResourceWAIT(char* message, int socketClient);

void sendResourceSIGNAL(char* message, int socketClient);
char* recvResourceWait(int socketClient);

char* recvResourceSignal(int socketClient);
char* moduleChar(MODULE module);
int findIndexOfString(char** array, size_t size, const char* value);
// typedef struct {
// 	int size;
// 	void* stream;
// } t_buffer;

// typedef struct {
// 	op_code operationCode;
// 	t_buffer* buffer;
// } t_package;

extern t_log* logger;

void* recvBuffer(int*, int);
void* serializePackage(t_package*, int);

//int waitClient(int);
int recvOperation(int);
//int createConection(char, char*);

char* recvMessage(int);
void finishProgram(int, t_log*, t_config*);
void closeConection(int);
void addPackage(t_package*, void*, int);
void sendPackage(t_package*, int);
void deletePackage(t_package*);
void sendMessage(char*, int);
int sendInt(int value, int socket);
int recvInt(int* value, int socket);
void* recvBufferInstruction(int* size, int socketClient);
char* recvInstruction(int socketClient);
char* recvMessage(int socketClient);
void parseStringToStringArray(char* str, char*** strArray, size_t* arraySize);
void parseStringToIntArray(char* str, int** intArray, size_t* arraySize) ;
char* getConfigValue(t_config*, char*);
char* trim(char* str);
void sendLine(const char* message, int socketClient);
t_list* recvPack(int);
t_log* initLogger(char*, char*, bool, t_log_level);
t_config* initConfig(char*);
void newBuffer(t_package*);
t_package* createPackage();

void sendPcb(pcb* pcb, int socketClient);
pcb* recvPcb(int clientFD);
void pcbSerialize(void* buffer, pcb* pcb);
pcb* pcbDeserialize(void* buffer);
void sendProcessToMemory(processToMemory* newProcess, int socketClient);
void processToMemorySerialize(void* buffer, processToMemory* newProcess);
processToMemory* memoryToProcessDeserialize(void* stream);
processToMemory* recvProcessToMemory(int socketClient);
uint32_t cpuPidAndCodStreamSize(int32_t request);
void cpuPidAndCodSerialize(void* buffer, int32_t request);
void instReqToMemorySerialize(void* buffer, instReqToMemory request);
int32_t cpuPidAndCodMemoryDeserialize(void* stream);
int32_t recvCpuPid(int socketClient);
void liberarRecursosDePID(Recursos_de_PID* recursosDePID);
instReqToMemory instReqToMemoryDeserialize (void* stream);
instReqToMemory recvInstReqToMemory(int socketClient);
Recursos_de_PID* crearRecursosDePID(int pid) ;
void agregarRecurso(Recursos_de_PID* recursosDePID, void* recurso);
solicitudFrameMemoria* NUEVA_SOLICITUD_FRAME_MEMORIA(uint32_t PID, int pagina);
void send_SOLICITUD_FRAME_MEMORIA(solicitudFrameMemoria* newRequestFRAME, int socketClient);
uint32_t SOLICITUD_FRAME_MEMORIA_StreamSize(solicitudFrameMemoria* newRequestFRAME);
void SOLICITUD_FRAME_MEMORIA_Serialize(void* buffer, solicitudFrameMemoria* newRequestFRAME);
solicitudFrameMemoria* recv_SOLICITUD_FRAME_MEMORIA(int socketClient);
solicitudFrameMemoria* SOLICITUD_FRAME_MEMORIA_Deserialize(void* stream);


RESIZE_params* NEW_RESIZE(int pid, int tamanioPagina);

void send_RESIZE (RESIZE_params* tamanioProceso, int socketClient);

uint32_t RESIZE_StreamSize(RESIZE_params* interface);

void RESIZE_Serialize(void* buffer, RESIZE_params* tamanioPagina);


RESIZE_params* recv_RESIZE(int socketClient);

RESIZE_params* RESIZE_Deserialize(void* stream);


char* recvSTDOUT(int socketClient);

char* detectModuleById(MODULE module);

char* intToChar(int num); // Convierte un numero en un char*. intToChar(19) = "19"

t_queue* splitString(char* str, const char* delimiter);

void sendInterrupt(char*, int);
void print_rectangle(const char* message);
char* getNextWord(char* operation, int* offset);
int getValue(char* buffer);

void send_WAIT_OR_SIGNAL(uint32_t result, int socketClient);

uint32_t recv_WAIT_OR_SIGNAL(int socketClient);


uint32_t send_WAIT_OR_SIGNAL_StreamSize();
void send_WAIT_OR_SIGNAL_Serialize(void* buffer, uint32_t value);

uint32_t WAIT_OR_SIGNAL_Deserialize(void* stream);


MOVE_IN_TO_MEMORY_params* NEW_MOVE_IN_TO_MEMORY(uint32_t pidMOVE_IN_TO_MEMORY, int length, uint32_t address) ;

void send_MOVE_IN_TO_MEMORY(MOVE_IN_TO_MEMORY_params* newSendIO, int socketClient);

uint32_t MOVE_IN_TO_MEMORY_StreamSize(MOVE_IN_TO_MEMORY_params* interface);

void MOVE_IN_TO_MEMORY_Serialize(void* buffer, MOVE_IN_TO_MEMORY_params* newSend);


MOVE_IN_TO_MEMORY_params* recv_MOVE_IN_TO_MEMORY(int socketClient);


MOVE_OUT_TO_MEMORY_params* MOVE_IN_TO_MEMORY_Deserialize(void* stream);

MOVE_OUT_TO_MEMORY_params* NEW_MOVE_OUT_TO_MEMORY(uint32_t pidMOVE_OUT_TO_MEMORY, int tamanioParaLeer, uint32_t address, uint32_t dataRegistro);

void send_MOVE_OUT_TO_MEMORY(MOVE_OUT_TO_MEMORY_params* newSendIO, int socketClient);

uint32_t MOVE_OUT_TO_MEMORY_StreamSize(MOVE_OUT_TO_MEMORY_params* interface);

void MOVE_OUT_TO_MEMORY_Serialize(void* buffer, MOVE_OUT_TO_MEMORY_params* newSend);


MOVE_OUT_TO_MEMORY_params* recv_MOVE_OUT_TO_MEMORY(int socketClient);


MOVE_OUT_TO_MEMORY_params* MOVE_OUT_TO_MEMORY_Deserialize(void* stream);
int isValidIntegerString(const char* str);
char* intToString(int value);


COPY_STRING_params* NEW_COPY_STRING(uint32_t pid_CS, uint32_t df, uint32_t direccionFIsicaORIGEN, uint32_t tamanio);

void send_COPY_STRING (COPY_STRING_params* newSendIO, int socketClient);

uint32_t COPY_STRING_StreamSize(COPY_STRING_params* interface) ;

void COPY_STRING_Serialize(void* buffer, COPY_STRING_params* newSendIO);

COPY_STRING_params* recv_COPY_STRING(int socketClient);

COPY_STRING_params* COPY_STRING_Deserialize(void* stream);

char* intToBinary8Bits(int num);
char* intToBinary32Bits(int num);
int binaryToInt8Bits(char* binary_str);
int binaryToInt32Bits(char* binary_str);
uint32_t cadenaANumero(const char *cadena);

#endif /* UTILS_H */
