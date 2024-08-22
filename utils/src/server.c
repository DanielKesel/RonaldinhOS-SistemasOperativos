#include "server.h"

t_log* logger;


int initServer(char* port)
{
	int socketServer;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(NULL, port, &hints, &servinfo);
    if (err < 0) {
        //log_error(logger, "Error generating socket. getaddrinfo: %s\n", gai_strerror(err)); // Devuelve el tipo de error cuando no se puede getaddrin
        //exit(EXIT_FAILURE);
        return -1;
    }

	// Creamos el socket de escucha del servidor
	
	socketServer = socket(servinfo->ai_family,
						  servinfo->ai_socktype,
						  servinfo->ai_protocol);

    if (setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
        error("setsockopt(SO_REUSEADDR) failed");

	// Asociamos el socket a un puerto
	int binding = bind(socketServer, servinfo->ai_addr, servinfo->ai_addrlen);
    if (binding == -1) {
        //log_error(logger, "Could not assign socket to some fd");
        //exit(EXIT_FAILURE);
        return -2;
    }

	// Escuchamos las conexiones entrantes
	int listening = listen(socketServer, SOMAXCONN);
    if (listening == -1) {
        //log_error(logger, "Cannot mark socket as listening to accept connection requests");
        //exit(EXIT_FAILURE);
        return -3;
    }

	freeaddrinfo(servinfo);
	//log_trace(logger, "Ready to listen to clients");

	return socketServer;
}


// DANI: Este handshake espera cualquier cliente, si no llega ninguno valido devuelve -1
clientConnection andHandshakeForAll(int socketServer)
{
    logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
    struct addrinfo hints, *servinfo;
    // Aceptamos un nuevo cliente
    int conexion_servidor_cliente = accept(socketServer, NULL, NULL);
    size_t bytes;

    clientConnection clientConn;
    clientConn.socket = -1; // Establecer por defecto como fallo

    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;
    
    
    // Dani:recibir el handshake del cliente
    bytes = recv(conexion_servidor_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (bytes <= 0) {
        if(conexion_servidor_cliente == -1){
            log_error(logger, "CUIDADO!! se falló el accept a un cliente!");
        } 
        else {
            log_error(logger, "Error al recibir el handshake del cliente\n");
            close(conexion_servidor_cliente); // Cierro

        } 
        return clientConn;
    }

    // Dani: proceso el handshake
    if (handshake == -1) {
        printf("Falló el handshake que envía el cliente.\n");
        bytes = send(conexion_servidor_cliente, &resultError, sizeof(int32_t), 0);
        close(conexion_servidor_cliente); // Cerrar la conexión
    } else {
        printf("Entra a SE CONECTO CLIENTE\n");
        char* aux = detectModuleById(handshake);  // VER SI SOLUCIONA MEMORY LEAK 150 BYTES (memoria)
        log_info(logger, "Se conectó el cliente: %s", aux);  // VER SI SOLUCIONA MEMORY LEAK 150 BYTES (memoria)
        printf("SALIO a SE CONECTO CLIENTE\n");
        bytes = send(conexion_servidor_cliente, &resultOk, sizeof(int32_t), 0);
        printf("SALIO DE SEND\n");
        clientConn.socket = conexion_servidor_cliente;
        printf("UTNSO\n");
        clientConn.client = handshake;
        printf("sale de handshake\n");

        free(aux);  // VER SI SOLUCIONA MEMORY LEAK 150 BYTES (memoria)
    }
    free(logger->program_name);  // VER SI SOLUCIONA MEMORY LEAK 27 BYTES
    free(logger);  // VER SI SOLUCIONA MEMORY LEAK 96 BYTES directos y 27 BYTES indirectos

    return clientConn;
}
// DANI: Este handshake espera un cliente especifico, si no llega ninguno valido devuelve -1
int andHandShakeForOne(int socketServer, MODULE requiredClient)
{
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	struct addrinfo hints, *servinfo;
	// Aceptamos un nuevo cliente

	int conexion_servidor_cliente = accept(socketServer, NULL, NULL);

	size_t bytes;

	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;
	//int *ptr = &resultOk; // Apuntador a la dirección de memoria de 'number'

    //printf("Dirección de memoria: %p\n", (void *)ptr); // Imprime la dirección de memoria
    //printf("Valor en esa dirección: %d\n", *ptr); // Imprime el valor almacenado en esa dirección

	bytes = recv(conexion_servidor_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (bytes <= 0) {
        if(conexion_servidor_cliente == -1){
            log_error(logger, "Servidor falló en aceptar al cliente: %s.\n", detectModuleById(handshake));
        } 
        else {
            log_error(logger, "Error al recibir el handshake del cliente: %s.\n", detectModuleById(handshake));
            close(conexion_servidor_cliente); // Cierro la conexión
        }
        return -1;
    }
	if (handshake == requiredClient) {
        bytes = send(conexion_servidor_cliente, &resultOk, sizeof(int32_t), 0);
        log_info(logger, "Se conectó el cliente: %s", detectModuleById(handshake));
        return conexion_servidor_cliente;
	} else {
        log_error(logger, "NOOOOOOOOO, NO ES %s, ES: %s !!!!!!\n", detectModuleById(requiredClient), detectModuleById(handshake));
        bytes = send(conexion_servidor_cliente , &resultError, sizeof(int32_t), 0);
		return -1;
	}
}


// int recvOperation(int socketClient)
// {
// 	int cod_op;
// 	if(recv(socketClient, &cod_op, sizeof(int), MSG_WAITALL) > 0)
// 		return cod_op;
// 	else
// 	{
// 		close(socketClient);
// 		return -1;
// 	}
// }

// void* recvBuffer(int* size, int socketClient)
// {
// 	void * buffer;

// 	recv(socketClient, size, sizeof(int), MSG_WAITALL);
// 	buffer = malloc(*size);
// 	recv(socketClient, buffer, *size, MSG_WAITALL);

// 	return buffer;
// }

// void recvMessage(int socketClient)
// {
// 	int size;
// 	char* buffer = recvBuffer(&size, socketClient);
// 	log_info(logger, "Message recived: %s", buffer);
// 	free(buffer);
// }

// t_list* recvPack(int socketClient)
// {
// 	int size;
// 	int desplazamiento = 0;
// 	void * buffer;
// 	t_list* valores = list_create();
// 	int tamanio;

// 	buffer = recvBuffer(&size, socketClient);
// 	while(desplazamiento < size)
// 	{
// 		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
// 		desplazamiento+=sizeof(int);
// 		char* valor = malloc(tamanio);
// 		memcpy(valor, buffer+desplazamiento, tamanio);
// 		desplazamiento+=tamanio;
// 		list_add(valores, valor);
// 	}
// 	free(buffer);
// 	return valores;
// }
