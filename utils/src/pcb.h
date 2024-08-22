#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "states.h"

typedef struct{
    uint32_t* PC;    // 32 Program Counter, indica la próxima instrucción a ejecutar.
    uint8_t* AX;     // 8  Registro Numérico de propósito general.
    uint8_t* BX;     // 8  Idem AX.
    uint8_t* CX;     // 8  Idem AX.
    uint8_t* DX;     // 8  Idem AX.
    uint32_t* EAX;   // 32 Registro Numérico de propósito general.
    uint32_t* EBX;   // 32 Idem EAX.
    uint32_t* ECX;   // 32 Idem EAX.
    uint32_t* EDX;   // 32 Idem EAX.
    uint32_t* SI;    // 32 Contiene la dirección lógica de memoria de origen desde donde se va a copiar un string.
    uint32_t* DI;    // 32 Idem SI.
} CpuRegister;

typedef struct
{
    char* reason;
    char* info;
} contextSwitch; // Motivo por el cual la CPU me pide cambio de contexto

typedef struct
{
    uint32_t pid;
    uint32_t quantum;
    CpuRegister cpuRegisters;
    stateProcess state;
    contextSwitch* context;
} pcb;

pcb* newPcb(int pid);
void initRegister(pcb* newPcb);

uint32_t pcbStreamSize(pcb* pcb);

void destroyPcb(pcb* pcb);
void destroyCpuRegister(CpuRegister reg);
void destroyContextSwitch(contextSwitch* cs);

char* pcbStateChar(stateProcess module);
#endif /* PCB_H */