#include "serialization.h"

t_buffer* bufferCreate(uint32_t streamSize) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = streamSize;
    buffer->stream = malloc(streamSize);
    return buffer;
}

t_package* packageCeate(op_code operationCode, uint32_t streamSize) {
    t_package* package = malloc(sizeof(t_package));
    
    package->operationCode = operationCode;
    package->buffer = bufferCreate(streamSize);
    return package;
}

void bufferDestroy(t_buffer* buffer) {
    free(buffer->stream);
    free(buffer);
}

void packageDestroy(t_package* package) {
    bufferDestroy(package->buffer);
	free(package);
}

void bufferAdd(t_buffer* buffer, void* data, uint32_t size, uint32_t *offset) {
    void* stream = buffer->stream;
    memcpy(stream + *(offset), data, size);
    *(offset) += size;
}

void* bufferRead(void* stream, uint32_t size, uint32_t *offset) {
    void* data = malloc(size);
    memcpy(data, stream + *(offset), size);
    *(offset) += size;
    return data;
}

void bufferAddInt32(t_buffer* buffer, int32_t* data, uint32_t *offset) {
    void* stream = buffer->stream;
    memcpy(stream + *(offset), data, sizeof(int32_t));
    *(offset) += sizeof(int32_t);
}

uint32_t bufferReadInt32(void* stream, int32_t* offset) {
    uint32_t data;
    memcpy(&data, stream + *(offset), sizeof(int32_t));
    *(offset) += sizeof(int32_t);
    return data;
}

void bufferAddUint32(t_buffer* buffer, uint32_t* data, uint32_t *offset) {
    void* stream = buffer->stream;
    memcpy(stream + *(offset), data, sizeof(uint32_t));
    *(offset) += sizeof(uint32_t);
}

uint32_t bufferReadUint32(void* stream, uint32_t* offset) {
    uint32_t data;
    memcpy(&data, stream + *(offset), sizeof(uint32_t));
    *(offset) += sizeof(uint32_t);
    return data;
}

void bufferAddUint8(t_buffer* buffer, uint8_t* data, uint32_t *offset) {
    void* stream = buffer->stream;
    memcpy(stream + *(offset), data, sizeof(uint8_t));
    *(offset) += sizeof(uint8_t);
}

uint8_t bufferReadUint8(void* stream, uint32_t* offset) {
    uint8_t data;
    memcpy(&data, stream + *(offset), sizeof(uint8_t));
    *(offset) += sizeof(uint8_t);
    return data;
}

void bufferAddString(t_buffer* buffer, char* string, uint32_t *offset) {
    void* stream = buffer->stream;
    uint32_t stringLength = strlen(string) + 1;
    bufferAddUint32(buffer, &stringLength, offset); // Antes de meter la palabra en el stream, tengo que meter su largo
    memcpy(stream + *(offset), string, stringLength);
    *(offset) += stringLength;
}

char* bufferReadString(void* stream, uint32_t* offset) {
    uint32_t stringLength = bufferReadUint32(stream, offset); // Antes de extraer el string, se que primero viene el largo del string
    char* data = malloc(stringLength);
    memcpy(data, stream + *(offset), stringLength);
    *(offset) += stringLength;
    return data;
}

void bufferAddCpuRegister(t_buffer* buffer, CpuRegister* registers, uint32_t *offset) {
    bufferAddUint32(buffer, registers->PC, offset);
    bufferAddUint8(buffer, registers->AX, offset);
    bufferAddUint8(buffer, registers->BX, offset);
    bufferAddUint8(buffer, registers->CX, offset);
    bufferAddUint8(buffer, registers->DX, offset);
    bufferAddUint32(buffer, registers->EAX, offset);
    bufferAddUint32(buffer, registers->EBX, offset);
    bufferAddUint32(buffer, registers->ECX, offset);
    bufferAddUint32(buffer, registers->EDX, offset);
    bufferAddUint32(buffer, registers->SI, offset);
    bufferAddUint32(buffer, registers->DI, offset);
}

// Lee un string y su longitud del buffer y avanza el offset
CpuRegister bufferReadCpuRegister(void* stream, uint32_t* offset) {
    CpuRegister registers;
    registers.PC = malloc(sizeof(uint32_t));
    registers.AX = malloc(sizeof(uint8_t));
    registers.BX = malloc(sizeof(uint8_t));
    registers.CX = malloc(sizeof(uint8_t));
    registers.DX = malloc(sizeof(uint8_t));
    registers.EAX = malloc(sizeof(uint32_t));
    registers.EBX = malloc(sizeof(uint32_t));
    registers.ECX = malloc(sizeof(uint32_t));
    registers.EDX = malloc(sizeof(uint32_t));
    registers.SI = malloc(sizeof(uint32_t));
    registers.DI = malloc(sizeof(uint32_t));
    uint32_t PC = bufferReadUint32(stream, offset);
    uint8_t AX = bufferReadUint8(stream, offset);
    uint8_t BX = bufferReadUint8(stream, offset);
    uint8_t CX = bufferReadUint8(stream, offset);
    uint8_t DX = bufferReadUint8(stream, offset);
    uint32_t EAX = bufferReadUint32(stream, offset);
    uint32_t EBX = bufferReadUint32(stream, offset);
    uint32_t ECX = bufferReadUint32(stream, offset);
    uint32_t EDX = bufferReadUint32(stream, offset);
    uint32_t SI = bufferReadUint32(stream, offset);
    uint32_t DI = bufferReadUint32(stream, offset);

    memcpy(registers.PC, &PC, sizeof(uint32_t));
    memcpy(registers.AX, &AX, sizeof(uint8_t));
    memcpy(registers.BX, &BX, sizeof(uint8_t));
    memcpy(registers.CX, &CX, sizeof(uint8_t));
    memcpy(registers.DX, &DX, sizeof(uint8_t));
    memcpy(registers.EAX, &EAX, sizeof(uint32_t));
    memcpy(registers.EBX, &EBX, sizeof(uint32_t));
    memcpy(registers.ECX, &ECX, sizeof(uint32_t));
    memcpy(registers.EDX, &EDX, sizeof(uint32_t));
    memcpy(registers.SI, &SI, sizeof(uint32_t));
    memcpy(registers.DI, &DI, sizeof(uint32_t));
    
    return registers;
}

void bufferAddCcontextSwitch(t_buffer* buffer, contextSwitch* contSwitch, uint32_t *offset) {
    bufferAddString(buffer, contSwitch->reason, offset);
    bufferAddString(buffer, contSwitch->info, offset);
}

contextSwitch* bufferReadcontextSwitch(void* stream, uint32_t* offset) {
    contextSwitch* contSwitch = malloc(sizeof(contextSwitch));

    contSwitch->reason = malloc(sizeof(char)*20);
    contSwitch->info = malloc(sizeof(char)*10);

    contSwitch->reason = bufferReadString(stream, offset);
    contSwitch->info = bufferReadString(stream, offset);
    return contSwitch;
}

void verStream(void* stream, uint32_t size) {
    char* valores = (char*)stream;
    printf("%s", valores);
    for (uint32_t i = 0; i < size; i++) {
        char caracter = valores[i]; // Tomar solo el byte menos significativo para imprimir como char
        printf("%c", caracter);
    }
    printf("\n");
}