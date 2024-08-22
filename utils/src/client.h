#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include "utils.h"


int createConnection(char* ip, char* port, MODULE moduleWhoIAm);
// void sendMessage(char* message, int socketCliente);
// t_package* createPackage(void);
// void addToPackage(t_package* package, void* value, int size);
// void sendPackage(t_package* package, int socketCliente);
void freeConection(int socketCliente);
//void deletePackage(t_package* package);
//void* serializePackage(t_package*, int)
//void crear_buffer(t_package*)
