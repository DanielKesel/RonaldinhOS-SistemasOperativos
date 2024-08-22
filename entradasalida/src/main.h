#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <readline/readline.h>
#include "utils.h"
#include "server.h"
#include "client.h"
#include "utils_io.h"
#include "IOFUNCTIONS.h"
pthread_t tid;
pthread_attr_t attr;
t_config* ioConfig;
sem_t sIO;


void* ConnRunner(void*);

char* IP_KERNEL;
char* PUERTO_KERNEL;
char* TIPO_INTERFAZ;
char* IP_MEMORIA;
char* TIEMPO_UNIDAD_TRABAJO;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
char* BLOCK_SIZE;
char* BLOCK_COUNT;
char* RETRASO_COMPACTACION;



char* genericPath;


char* logString; // Cada vez que se ejecute alguna de las siguientes funcines, lo que se guarde en el log, primero se guarda en esta variable y despues se libera
// Log para todas las interfaces
char* interfaceName;
char* pathConfig;

char * path_blocks;
char * path_bitmap;
char * path_metadata;
int32_t fileBlocksSize;
int32_t fileBitmapSize;
t_bitarray* bitmap;
t_bitarray* blocks;
t_list* globalOpenFiles;

char* listOpenFiles;



int createInterface();


void logOperation(int pid, char* operation);

void sendPid(uint32_t pid, int socket);
void print_rectangle(const char* message);
int endsWithSlash(char *str);

// Log para la interfaz DialFS
void logCreateFile(int pid, char* fileName);
void logDeleteFile(int pid, char* fileName);
void logTruncateFile(int pid, char* fileName, int size);
void logReadFile(int pid, char* fileName, int size, char* ptr);
void logWriteFile(int pid, char* fileName, int size, char* ptr);
void logStartCompaction(int pid);
void logEndCompaction(int pid);
