#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pcb.h"

typedef enum {
	MESSAGE,
	PACKAGE,
	PCB,
	PROCESSTOMEMORY,
	REQUESTINSTRUCTIONS,
	INTERRUPTBYQUANTUM,
	IOFUNCTION,
	RESIZE,
	SLEEP,
	READ,
	IO_STDIN_READ_OP,
	WRITE,
	FS_CREATE,
	FS_DELETE,
	FS_TRUNCATE,
	FS_WRITE,
	FS_READ,
	TAMANIO_PAGINA,
	SOLICITUD_FRAME,
	RESOURCE,
	WAIT,
	SIGNAL,
	BLOCK,
	EMPTY_INSTANCE,
	CONTINUE,
	MOVE_IN_OP,
	MOVE_OUT_OP,
	COPY_STRING,
	FINISHED_IO,
	FINISHED_IO_ERROR
}op_code;

typedef struct {
	uint32_t size; // Tamaño del payload
	//uint32_t* offset; // Desplazamiento dentro del payload
	void* stream; // Payload
} t_buffer;

typedef struct {
	op_code operationCode;
	t_buffer* buffer;
} t_package;


// Crea un buffer vacío de tamaño size y offset 0
t_buffer* bufferCeate(uint32_t size);

// Crea el packete con el tamaño de buffer establecido
t_package* packageCeate(op_code operationCode, uint32_t bufferSize);

// Libera la memoria asociada al buffer
void bufferDestroy(t_buffer* buffer);

// Agrega un stream al buffer en la posición actual y avanza el offset
void bufferAdd(t_buffer* buffer, void* data, uint32_t size, uint32_t *offset);

// Guarda size bytes del principio del buffer en la dirección data y avanza el offset
void* bufferRead(void* stream, uint32_t size, uint32_t* offset);

// Agrega un int32_t al buffer
void bufferAddInt32(t_buffer* buffer, int32_t* data, uint32_t *offset);

// Lee un int32_t del buffer y avanza el offset
uint32_t bufferReadInt32(void* stream, int32_t* offset);

// Agrega un uint32_t al buffer
void bufferAddUint32(t_buffer* buffer, uint32_t* data, uint32_t *offset);

// Lee un uint32_t del buffer y avanza el offset
uint32_t bufferReadUint32(void* stream, uint32_t* offset);

// Agrega un uint8_t al buffer
void bufferAddUint8(t_buffer* buffer, uint8_t* data, uint32_t *offset);

// Lee un uint8_t del buffer y avanza el offset
uint8_t bufferReadUint8(void* stream, uint32_t* offset);

// Agrega string al buffer con un uint32_t adelante indicando su longitud
void bufferAddString(t_buffer* buffer, char* string, uint32_t *offset);

// Lee un string y su longitud del buffer y avanza el offset
char* bufferReadString(void* stream, uint32_t* offset);

// Agrega CpuRegister al buffer y avanza el offset
void bufferAddCpuRegister(t_buffer* buffer, CpuRegister* registers, uint32_t *offset);

// Lee un CpuRegister y avanza el offset
CpuRegister bufferReadCpuRegister(void* stream, uint32_t* offset);

// Agrega contextSwitch al buffer y avanza el offset
void bufferAddCcontextSwitch(t_buffer* buffer, contextSwitch* contSwitch, uint32_t *offset);

// Lee un contextSwitch y avanza el offset
contextSwitch* bufferReadcontextSwitch(void* stream, uint32_t* offset);


void verStream(void* stream, uint32_t size);

#endif /* SERIALIZATION_H */