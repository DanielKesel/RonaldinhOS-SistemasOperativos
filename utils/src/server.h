#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#define BUF_SIZE 1024
#include "utils.h"

extern t_log* logger;

typedef struct {
    int socket;
    int client;
} clientConnection;

//void* recvBuffer(int*, int);

int initServer(char*);

clientConnection andHandshakeForAll(int);
int andHandShakeForOne(int, MODULE);
//t_list* recvPack(int);
//void recvMessage(int);
//int recvOperation(int);
