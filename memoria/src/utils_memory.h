#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/hello.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include "utils.h"
#include <fcntl.h>
#include "math.h"
#define STRINGIFY(x) #x
#define MAX_LINE_LENGTH 200
typedef struct
{
    uint32_t pid;
    char* path;
} process;


// typedef enum {
//     SET,
//     SUM,
//     SUB,
//     MOV_IN,
//     MOV_OUT,
//     RESIZE,
//     JNZ,
//     COPY_STRING,
//     IO_GEN_SLEEP,
//     IO_STDIN_READ,
//     IO_STDOUT_WRITE,
//     IO_FS_CREATE,
//     IO_FS_DELETE,
//     IO_FS_TRUNCATE,
//     IO_FS_WRITE,
//     IO_FS_READ
// } InstructionType;

// typedef struct {
//     InstructionType type;
//     int numOperands;
//     int* operands;
// } Instruction;

extern t_log* log_memory;
extern char* logMemoryString;

extern t_config* memoryConfig ;
extern char* memoryPort;
extern char* lengthMemory;
extern char* lengthPage;
extern char* pathInstructions;
extern char* answerDelay;

void inicializarConfig();
char** appetiteForInstruction(char*);
int waitProgramCounterStrike(int* programCounter, int socketClient);
int sendInstructionToCpu(const char *buffer, int socket);
int waitRequestOfCPU(int* value, int socketClient);      
int waitProgramCounterStrike(int* value, int socketClient);
void readArrayOfInstructions(char** arrayOfInstructions);
int sendInstructionToCpu(const char *buffer, int socket);

int bitsToBytes(int);

void logNewPageTable(int pid, int size);
void logDestroyPageTable(int pid, int size);
void logPageTableAccess(int pid, int page, int marco);
void logProcessExpansion(int pid, int actualSize, int newSize);
void logProcessReduction(int pid, int actualSize, int newSize);
void logAccessToUserSpaceRead(int pid, int dir, char* size);
void logAccessToUserSpaceWrite(int pid, int dir, char* size);
#endif /* MEMORY_UTILS_H */