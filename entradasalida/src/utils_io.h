#ifndef UTILS_IO_H
#define UTILS_IO_H
#include <stdint.h>
#include "utils.h"
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <semaphore.h>

typedef struct {
    char* fileName;
    uint32_t pid;
} fcb;

// Struct que se usa en Compactación
typedef struct {
    char* fileName;
    uint32_t block;
    uint32_t size;
} Metadata;

extern t_list* globalOpenFiles;
void inicializarConfig();
int createInterface();
void crear_archivo_bloques();
void levantar_archivo_bitarray();
void levantar_archivo_bloques();
void ejecutar_fs_create(char* fileName);

int obtener_indice_bloque_libre();


extern t_config* ioConfig;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* TIPO_INTERFAZ;
extern char* IP_MEMORIA;
extern char* TIEMPO_UNIDAD_TRABAJO;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern char* BLOCK_SIZE;
extern char* BLOCK_COUNT;
extern char* RETRASO_COMPACTACION;
extern char* genericPath;
extern char* logString; 
extern char* interfaceName;
extern char* pathConfig;
extern char * path_blocks;
extern char * path_bitmap;
extern char * path_metadata;
extern int32_t fileBlocksSize;
extern int32_t fileBitmapSize;

extern t_bitarray* bitmap;
extern t_bitarray* blocks;

fcb* createFcb(char* fileName, uint32_t pid);
void deleteFcb(fcb* fcbToDelete);

void createFileBlocks();
void initializeFileBitarray();

void fsCreate(char* fileName, uint32_t pid);
int fsTruncate(char* fileName, uint32_t newSize); // newSize es en Bits
void fsDelete(char* fileName);
char* fsRead(char* fileName, uint32_t pointer, uint32_t dataSize); // dataSize es en Bytes
void fsWrite(char* fileName, uint32_t pointer, char* data);

int getFreeBlockIndex();
fcb* findFcbInGlobalOpenFiles(char* fileName);

void createMetadata(char* fileName);
void modifyMetadata(char* fileName, uint32_t newSize, char* key);
//t_file* createFile(char* fileName, t_config* metadata);

void assignBit(t_bitarray* bitarray, int blockIndex);
void deallocateBit(t_bitarray* bitarray, int blockIndex);
int nextBlocksAvaiable(uint32_t startIndex, uint32_t blockCount);

void msyncBitmap();
void msyncBlocks();
int closeFCB(fcb* fbcToClose, uint32_t pid);
int canRelocate(char* fileName, uint32_t necessaryBlocks);
int canRelocateAtTheEnd(char* fileName, uint32_t necessaryBlocks);
void realocate(char* fileName, uint32_t newInitialBlock, uint32_t newSize);

void compaction();
t_list* listMetadata();
int isMetadata(char* fileName);
char* fileNameWithoutExtens(char* fullFileName);


uint32_t blocksPerBits(uint32_t bytes);

uint32_t getToMetadata(char* fileName, char* key);
char* pathMetadata(char* fileName);

int metadataExist(char* fileName);

char* uint32ToChar(uint32_t);

int directory_exists(char* path);
int create_directory(char* path);
int ensure_directory(char* path);

#endif /* UTILS_IO_H */