#ifndef IOFUNCTIONS_H
#define IOFUNCTIONS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"



typedef struct {
	int socket;
	char* name;
} io;




void sendFunctionIo(char* message, int socketClient);
void* recvBuffer(int* size, int socketClient);
char* recvIo(int socketClient);
void printFormattedText(char* text);

//-----------------------------GENERIC-------------------------------------//

//int32_t recv_IO_GEN_SLEEP(int socketClient);
void send_IO_GEN_SLEEP(IO_GEN_SLEEP_params* newSendIO, int socketClient);
IO_GEN_SLEEP_params* NEW_IO_GEN_SLEEP(uint32_t pid, uint32_t timeToSleep);
void IO_GEN_SLEEP_Serialize(void* buffer, IO_GEN_SLEEP_params* newSendIO);
IO_GEN_SLEEP_params* IO_GEN_SLEEP_Deserialize(void* stream);
IO_GEN_SLEEP_params* recv_IO_GEN_SLEEP(int socketClient);
uint32_t IO_GEN_SLEEP_StreamSize(IO_GEN_SLEEP_params* sleepGEN);
//-----------------------------STDIN-------------------------------------//

//IO_STDIN_READ* NEW_IO_STDIN_READ(uint32_t DF, uint32_t length) ;
void send_IO_STDIN_READ(IO_STDIN_READ* newSendIO, int socketClient);
uint32_t NEW_IO_STDIN_READ_StreamSize(IO_STDIN_READ* newIOsend);
void IO_STDIN_READ_Serialize(void* buffer, IO_STDIN_READ* newSendIO);
IO_STDIN_READ* NEW_IO_STDIN_READ(uint32_t pid, uint32_t DF, uint32_t length);
IO_STDIN_READ* IO_STDIN_READ_Deserialize(void* stream) ;
IO_STDIN_READ* recv_IO_STDIN_READ(int socketClient);
IO_STDIN_READ_WRITE_MEMORY* NEW_IO_STDIN_READ_WRITE_MEMORY(uint32_t DF, char* text, uint32_t pid);
void IO_STDIN_READ_WRITE_MEMORY_Serialize(void* buffer, IO_STDIN_READ_WRITE_MEMORY* newSendIo);
IO_STDIN_READ_WRITE_MEMORY* recv_IO_STDIN_READ_WRITE_MEMORY(int socketClient);
void send_IO_STDIN_READ_WRITE_MEMORY(IO_STDIN_READ_WRITE_MEMORY* newSendIo, int socketClient);



//-----------------------------STDOUT-------------------------------------//

IO_STDOUT_WRITE* NEW_IO_STDOUT_WRITE(uint32_t pid, uint32_t DF, uint32_t length);
uint32_t NEW_IO_STDOUT_WRITE_StreamSize(IO_STDOUT_WRITE* newIOsend);
void send_IO_STDOUT_WRITE(IO_STDOUT_WRITE* newSendIO, int socketClient);
void IO_STDOUT_WRITE_Serialize(void* buffer, IO_STDOUT_WRITE* newSendIO);
IO_STDOUT_WRITE* IO_STDOUT_WRITE_Deserialize(void* stream);
IO_STDOUT_WRITE* recv_IO_STDOUT_WRITE(int socketClient);


//-----------------------------FS_CREATE------------------------------------//
IO_FS_CREATE_params* NEW_IO_FS_CREATE(uint32_t pid, char* fileName_FS);
IO_FS_CREATE_params* recv_IO_FS_CREATE(int socketClient);
IO_FS_CREATE_params* IO_FS_CREATE_Deserialize(void* stream);
void send_FS_CREATE (IO_FS_CREATE_params* newSendIO, int socketClient);
void IO_FS_CREATE_Serialize(void* buffer, IO_FS_CREATE_params* newSendIO);
uint32_t IO_FS_CREATE_StreamSize(IO_FS_CREATE_params* interface);


//-----------------------------FS_DELETE------------------------------------//
IO_FS_DELETE_params* NEW_IO_FS_DELETE(uint32_t pid, char* fileName_FS);
IO_FS_DELETE_params* recv_IO_FS_DELETE(int socketClient);
IO_FS_DELETE_params* IO_FS_DELETE_Deserialize(void* stream);
void send_FS_DELETE (IO_FS_DELETE_params* newSendIO, int socketClient);
void IO_FS_DELETE_Serialize(void* buffer, IO_FS_DELETE_params* newSendIO);
uint32_t IO_FS_DELETE_StreamSize(IO_FS_DELETE_params* interface);


// ----------------------------------FS_TRUNCATE---------------------------------
IO_FS_TRUNCATE_params* NEW_IO_FS_TRUNCATE(uint32_t pid_FS, char* fileName_FS, uint32_t newLength);
void send_FS_TRUNCATE (IO_FS_TRUNCATE_params* newSendIO, int socketClient);
uint32_t IO_FS_TRUNCATE_StreamSize(IO_FS_TRUNCATE_params* interface);
void IO_FS_TRUNCATE_Serialize(void* buffer, IO_FS_TRUNCATE_params* newSendIO);
IO_FS_TRUNCATE_params* recv_IO_FS_TRUNCATE(int socketClient);
IO_FS_TRUNCATE_params* IO_FS_TRUNCATE_Deserialize(void* stream);

// ----------------------------------FS_WRITE---------------------------------------------------

IO_FS_WRITE_params* NEW_IO_FS_WRITE(uint32_t pid_FS, char* fileName_FS, uint32_t address, uint32_t newLength, uint32_t newPointer);
void send_FS_WRITE(IO_FS_WRITE_params* newSendIO, int socketClient) ;
uint32_t IO_FS_WRITE_StreamSize(IO_FS_WRITE_params* interface);
void IO_FS_WRITE_Serialize(void* buffer, IO_FS_WRITE_params* newSendIO);
IO_FS_WRITE_params* recv_IO_FS_WRITE(int socketClient);
IO_FS_WRITE_params* IO_FS_WRITE_Deserialize(void* stream);

// ----------------------------------FS_READ---------------------------------------------------
IO_FS_READ_params* NEW_IO_FS_READ(uint32_t pid_FS, char* fileName_FS, uint32_t address, uint32_t newLength, uint32_t newPointer);
void send_FS_READ(IO_FS_WRITE_params* newSendIO, int socketClient);
uint32_t IO_FS_READ_StreamSize(IO_FS_READ_params* interface);
void IO_FS_READ_Serialize(void* buffer, IO_FS_READ_params* newSendIO);
IO_FS_READ_params* recv_IO_FS_READ(int socketClient);
IO_FS_READ_params* IO_FS_READ_Deserialize(void* stream) ;
IO_FS_READ_TO_MEMORY_params* NEW_IO_FS_READ_TO_MEMORY(uint32_t pid, uint32_t length, uint32_t address, char* newWord);
void send_FS_READ_TO_MEMORY(IO_FS_READ_TO_MEMORY_params* newSendIO, int socketClient);
uint32_t IO_FS_READ_TO_MEMORY_StreamSize(IO_FS_READ_TO_MEMORY_params* interface); 
void IO_FS_READ_TO_MEMORY_Serialize(void* buffer, IO_FS_READ_TO_MEMORY_params* newSendIO);
IO_FS_READ_TO_MEMORY_params* recv_IO_FS_READ_TO_MEMORY_(int socketClient);
IO_FS_READ_TO_MEMORY_params* IO_FS_READ_TO_MEMORY_Deserialize(void* stream);



#endif /* IOFUNCTIONS_H */