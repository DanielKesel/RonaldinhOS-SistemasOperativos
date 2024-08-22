#include "client.h"
// void* serializePackage(t_package* package, int bytes)
// {
// 	void * magic = malloc(bytes);
// 	int offset = 0;

// 	memcpy(magic + offset, &(package->operationCode), sizeof(int));
// 	offset+= sizeof(int);
// 	memcpy(magic + offset, &(package->buffer->size), sizeof(int));
// 	offset+= sizeof(int);
// 	memcpy(magic + offset, package->buffer->stream, package->buffer->size);
// 	offset+= package->buffer->size;

// 	return magic;
// }

// Dani: Modifico esta fución para que reciba como parametro que cliente es.
int createConnection(char *ip, char *port, MODULE selfModule)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, port, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);;

	// Ahora que tenemos el socket, vamos a conectarlo

	int connection = connect(socket_cliente , server_info->ai_addr, server_info->ai_addrlen);
	if (connection == -1) {
		return -1; // No se pudo conectar
	}

	size_t bytes;

	int32_t handshake = selfModule;
	int32_t result;

	bytes = send(socket_cliente, &handshake, sizeof(int32_t), 0);
	bytes = recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);
	

	if (result == 0) {
		printf("Handshake OK\n");
		freeaddrinfo(server_info);
		return socket_cliente;
	} else {
		printf("Handshake ERROR\n");
		close(socket_cliente);
		freeaddrinfo(server_info);
		return -1;
	}
}

// void sendMessage(char* message, int socketCliente)
// {
// 	t_package* package = malloc(sizeof(t_package));

// 	package->operationCode = MESSAGE;
// 	package->buffer = malloc(sizeof(t_buffer));
// 	package->buffer->size = strlen(message) + 1;
// 	package->buffer->stream = malloc(package->buffer->size);
// 	memcpy(package->buffer->stream, message, package->buffer->size);

// 	int bytes = package->buffer->size + 2*sizeof(int);

// 	void* sendTo = serializePackage(package, bytes);

// 	send(socketCliente, sendTo, bytes, 0);

// 	free(sendTo);
// 	deletePackage(package);
// }

// void crear_buffer(t_package* paquete)
// {
// 	paquete->buffer = malloc(sizeof(t_buffer));
// 	paquete->buffer->size = 0;
// 	paquete->buffer->stream = NULL;
// }

// t_package* createPackage(void)
// {
// 	t_package* package = malloc(sizeof(t_package));
// 	package->operationCode = PACKAGE;
// 	crear_buffer(package);
// 	return package;
// }

// void addToPackage(t_package* package, void* value, int size)
// {
// 	package->buffer->stream = realloc(package->buffer->stream, package->buffer->size + size + sizeof(int));

// 	memcpy(package->buffer->stream + package->buffer->size, &size, sizeof(int));
// 	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), value, size);

// 	package->buffer->size += size + sizeof(int);
// }

// void sendPackage(t_package* package, int socketCliente)
// {
// 	int bytes = package->buffer->size + 2*sizeof(int);
// 	void* toSend = serializePackage(package, bytes);

// 	send(socketCliente, toSend, bytes, 0);

// 	free(toSend);
// }

// void deletePackage(t_package* package)
// {
// 	free(package->buffer->stream);
// 	free(package->buffer);
// 	free(package);
// }

void freeConection(int socketCliente)
{
	close(socketCliente);
}
