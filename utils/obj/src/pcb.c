#include "pcb.h"

pcb* newPcb(int pid) {
    pcb* newPcb = malloc(sizeof(pcb));
    newPcb->context = malloc(sizeof(contextSwitch));
    newPcb->context->reason = malloc(sizeof(char)*50);
    newPcb->context->info = malloc(sizeof(char)*20);

    if (newPcb  == NULL) {
        return NULL; // Error
    }
    // (*pid)++;
    newPcb->pid = pid;
    //newPcb->programCounter = 0;
    newPcb->quantum = 0;
    initRegister(newPcb);
    newPcb->state = NEW;
    // newPcb->string = malloc(sizeof(char)*10);
    strcpy(newPcb->context->reason, "");
    strcpy(newPcb->context->info, "");
    return newPcb;
}

void initRegister(pcb* newPcb){

    newPcb->cpuRegisters.PC = malloc(sizeof(uint32_t));
    newPcb->cpuRegisters.AX = (uint8_t*)malloc(sizeof(uint8_t));
    newPcb->cpuRegisters.BX = (uint8_t*)malloc(sizeof(uint8_t));
    newPcb->cpuRegisters.CX = (uint8_t*)malloc(sizeof(uint8_t));
    newPcb->cpuRegisters.DX = (uint8_t*)malloc(sizeof(uint8_t));
    newPcb->cpuRegisters.EAX = malloc(sizeof(uint32_t));
    newPcb->cpuRegisters.EBX = malloc(sizeof(uint32_t));
    newPcb->cpuRegisters.ECX = malloc(sizeof(uint32_t));
    newPcb->cpuRegisters.EDX = malloc(sizeof(uint32_t));
    newPcb->cpuRegisters.SI = malloc(sizeof(uint32_t));
    newPcb->cpuRegisters.DI = malloc(sizeof(uint32_t));

    *newPcb->cpuRegisters.PC = 0;
    *newPcb->cpuRegisters.AX = 0;
    *newPcb->cpuRegisters.BX = 0;
    *newPcb->cpuRegisters.CX = 0;
    *newPcb->cpuRegisters.DX = 0;
    *newPcb->cpuRegisters.EAX = 0;
    *newPcb->cpuRegisters.EBX = 0;
    *newPcb->cpuRegisters.ECX = 0;
    *newPcb->cpuRegisters.EDX = 0;
    *newPcb->cpuRegisters.SI = 0;
    *newPcb->cpuRegisters.DI = 0;
}

uint32_t pcbStreamSize(pcb* pcb) {
    uint32_t pid;
    uint32_t quantum;
    CpuRegister cpuRegisters;
    stateProcess state;
    contextSwitch context;

    uint32_t size = 0;
    size = sizeof(pcb->pid) +
           sizeof(pcb->quantum) +
           sizeof(pcb->cpuRegisters.PC) +
           sizeof(pcb->cpuRegisters.AX) +
           sizeof(pcb->cpuRegisters.BX) +
           sizeof(pcb->cpuRegisters.CX) +
           sizeof(pcb->cpuRegisters.DX) +
           sizeof(pcb->cpuRegisters.EAX) +
           sizeof(pcb->cpuRegisters.EBX) +
           sizeof(pcb->cpuRegisters.ECX) +
           sizeof(pcb->cpuRegisters.EDX) +
           sizeof(pcb->cpuRegisters.SI) +
           sizeof(pcb->cpuRegisters.DI) +
           sizeof(uint32_t) + // Tamaño char reason
           strlen(pcb->context->reason) +
           sizeof(uint32_t) + // Tamaño char io
           strlen(pcb->context->info);
    return size;
}


void destroyPcb(pcb* pcb) {
    destroyCpuRegister(pcb->cpuRegisters);
    destroyContextSwitch(pcb->context);
    free(pcb);
}

void destroyCpuRegister(CpuRegister reg) {
    free(reg.PC);
    free(reg.AX);
    free(reg.BX);
    free(reg.CX);
    free(reg.DX);
    free(reg.EAX);
    free(reg.EBX);
    free(reg.ECX);
    free(reg.EDX);
    free(reg.SI);
    free(reg.DI);
}

void destroyContextSwitch(contextSwitch* cs) {
    free(cs->reason);
    free(cs->info);
    free(cs);
}

char* pcbStateChar(stateProcess module) {
	switch (module)
	{
	case NEW:
		return "NEW";
	break;
	case READY:
		return "READY";
	break;
	case BLOCKED:
		return "BLOCKED";
	break;
	case EXEC:
		return "EXEC";
	break;
    case EXIT:
		return "EXIT";
	break;

	default:
		break;
	}
}