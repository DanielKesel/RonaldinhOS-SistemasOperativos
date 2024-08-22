#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include "utils.h"

extern t_log* log_cpu;
extern char* logCpuString; // Cada vez que se ejecute alguna de las siguientes funcines, lo que se guarde en el log, primero se guarda en esta variable y despues se libera

void logFetchInstruction(int pid, int programCounter);
void logExecutedInstruction(int pid, char* instruction, char* params);
void logTlbHit(int pid, int page);
void logTlbMiss(int pid, int page);
void logGetFrame(int pid, int page, int marco);
void logReadMemory(int pid, char* dir, char* value);
void logWriteMemory(int pid, char* dir, char* value);