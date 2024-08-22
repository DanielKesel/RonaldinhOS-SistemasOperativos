#include "utils_cpu.h"

void logFetchInstruction(int pid, int programCounter) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	char* programCounterStr = intToChar(programCounter);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - FETCH - Program Counter: ");
	strcat(logCpuString, programCounterStr);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
	free(programCounterStr);
}

void logExecutedInstruction(int pid, char* instruction, char* params) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - Ejecutando: ");
	strcat(logCpuString, instruction);
	strcat(logCpuString, " - ");
	strcat(logCpuString, params);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
}

void logTlbHit(int pid, int page) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	char* pageStr = intToChar(page);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - TLB HIT - Pagina: ");
	strcat(logCpuString, pageStr);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
	free(pageStr);
}

void logTlbMiss(int pid, int page) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	char* pageStr = intToChar(page);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - TLB MISS - Pagina: ");
	strcat(logCpuString, pageStr);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
	free(pageStr);
}

void logGetFrame(int pid, int page, int frame) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	char* pageStr = intToChar(page);
	char* frameStr = intToChar(frame);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - OBTENER MARCO - Página: ");
	strcat(logCpuString, pageStr);
	strcat(logCpuString, " - Marco: ");
	strcat(logCpuString, frameStr);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
	free(pageStr);
	free(frameStr);
}

void logReadMemory(int pid, char* dir, char* value) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - Acción: LEER - Dirección Física: ");
	strcat(logCpuString, dir);
	strcat(logCpuString, " - Valor: ");
	strcat(logCpuString, value);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
    free(dir);
    free(value);
}

void logWriteMemory(int pid, char* dir, char* value) {
	logCpuString = malloc(sizeof(char) *100);
	char* pidStr = intToChar(pid);
	strcpy(logCpuString, "PID: ");
	strcat(logCpuString, pidStr);
	strcat(logCpuString, " - Acción: ESCRIBIR - Dirección Física: ");
	strcat(logCpuString, dir);
	strcat(logCpuString, " - Valor: ");
	strcat(logCpuString, value);
	log_info(log_cpu, logCpuString);
	free(logCpuString);
	free(pidStr);
    free(dir);
    free(value);
}