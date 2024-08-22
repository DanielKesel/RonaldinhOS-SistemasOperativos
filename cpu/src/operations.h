#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdint.h>
#include "utils.h"
#include "server.h"
#include "IOFUNCTIONS.h"
#include "MMU.h"
#include <semaphore.h>

int executeOperations(char*, pcb*);
void* getRegister(char*);
extern t_config *config_CPU;
void SET(char*, int);
void MOV_IN(char*, char*);
void MOV_OUT(char*, char*);
void SUM(char*, char*);
void SUB(char*, char*);
void JNZ(char*, int);
void COPY_STRING_(int);
void RE_SIZE(int);
void IO_GEN_SLEEP(char* iOInstruction);
void IO_STDIN_READ_FN(char* iOInstruction);

//char* uint32ToChar(uint32_t toTransform);
char* uint8ToChar(uint8_t toTransform);
uint32_t dato_reconstruido(uint32_t primera, uint32_t segunda, int bytes_primera, int bytes_segunda);
uint32_t leer_y_guardar_de_dos_paginas(uint32_t dir_logica_P, uint32_t pid);
uint32_t charToUint32(char* str);
char* uint32ToChar(uint32_t regValue);

extern clientConnection clientCpuDispatch; 
extern int memoryConnection;
extern int kernelSocket;
extern int32_t pagesLength;
extern int cod_op_resource;
extern sem_t sExecuteCodeOpResources;
#endif /* OPERATIONS_H */