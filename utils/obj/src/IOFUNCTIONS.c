#include "IOFUNCTIONS.h"

//Esta función envía que es lo que se quiere realizar sobre una IO (especificando QUE IO es y que se requiere hacer) al kernel
void sendFunctionIo(char* message, int socketClient) {

	t_package* package = malloc(sizeof(t_package));

	package->operationCode = IOFUNCTION;
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = strlen(message) + 1;
	package->buffer->stream = malloc(package->buffer->size);
	memcpy(package->buffer->stream, message, package->buffer->size);

	int bytes = package->buffer->size + 2*sizeof(int);

	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	deletePackage(package);
}



//Esta función recibe la petición que se requiere hacer sobre una io devolviendo una instrucción por ejemplo 'IO_GEN_SLEEP IO1 2'
char* recvIo(int socketClient)
{
	int size;
	char* buffer = recvBuffer(&size, socketClient);
	int32_t result = 0;
	send(socketClient, &result, sizeof(int32_t), 0);
	log_info(logger, "Function for IO recived: %s", buffer);
	return buffer;
}

void send_IO_GEN_SLEEP(IO_GEN_SLEEP_params* newSendIO, int socketClient) {
	// t_package* package = malloc(sizeof(t_package));

	// package->operationCode = SLEEP;
	// package->buffer = malloc(sizeof(t_buffer));
	// package->buffer->size = sizeof(timeToSleep);
	// package->buffer->stream = malloc(package->buffer->size);
	// memcpy(package->buffer->stream, &timeToSleep, package->buffer->size);

	// int bytes = package->buffer->size + 2*sizeof(int);

	// void* toSend = serializePackage(package, bytes);

	// send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	// free(toSend);
	// deletePackage(package);

	t_package* package = packageCeate(SLEEP, IO_GEN_SLEEP_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_GEN_SLEEP_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}


IO_GEN_SLEEP_params* recv_IO_GEN_SLEEP(int socketClient)
{
    // int32_t value;
    // recv(socketClient, &value, sizeof(int), MSG_WAITALL);
    // int32_t timeToSleep = value;
    // int32_t result = 0;
    // log_info(logger, "Time to sleep received: %d", timeToSleep);
    // return timeToSleep;
	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_GEN_SLEEP_params* IO_GEN_SLEEP_Recibed = IO_GEN_SLEEP_Deserialize(stream);
	free(stream);
	return IO_GEN_SLEEP_Recibed;
}



uint32_t IO_GEN_SLEEP_StreamSize(IO_GEN_SLEEP_params* sleepGEN) {
    uint32_t size = 0;
    size += sizeof(uint32_t);
    size += sizeof(uint32_t);
    return size;
}

IO_GEN_SLEEP_params* NEW_IO_GEN_SLEEP(uint32_t pid, uint32_t timeToSleep) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_GEN_SLEEP_params* newIOsend = malloc(sizeof(IO_GEN_SLEEP_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend->pid = pid;
    newIOsend->timeToSleep = timeToSleep;
    
    return newIOsend;
}

void IO_GEN_SLEEP_Serialize(void* buffer, IO_GEN_SLEEP_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	uint32_t timeToSleep = newSendIO -> timeToSleep;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &timeToSleep, &offset);
}

IO_STDIN_READ* NEW_IO_STDIN_READ(uint32_t pid, uint32_t DF, uint32_t length) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_STDIN_READ* newIOsend = malloc(sizeof(IO_STDIN_READ));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend->pid = pid;
    newIOsend->physicalAdress = DF;
	newIOsend->lengthToReadOrWrite = length;
    
    return newIOsend;
}

IO_STDIN_READ_WRITE_MEMORY* NEW_IO_STDIN_READ_WRITE_MEMORY(uint32_t DF, char* text, uint32_t pid) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_STDIN_READ_WRITE_MEMORY* newIOsend = malloc(sizeof(IO_STDIN_READ_WRITE_MEMORY));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
	newIOsend->pid = pid;
    newIOsend->physicalAdress = DF;
	newIOsend->textToMemory = text;
    
    return newIOsend;
}

uint32_t NEW_IO_STDIN_READ_StreamSize(IO_STDIN_READ* newIOsend) {
    uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del Pid
    size += sizeof(uint32_t); // Tamaño del DF
    size += sizeof(uint32_t); // Tamaño de tamaño de lectura o escritura
    return size;

}

uint32_t IO_STDIN_READ_WRITE_MEMORY_StreamSize(IO_STDIN_READ_WRITE_MEMORY* newIOsend) {
	
    uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del DF
    size += sizeof(uint32_t); // Tamaño de la longitud del texto
	size += sizeof(uint32_t); // Tamaño del  uint32_t pid;
    size += strlen(newIOsend -> textToMemory) + 1; // Tamaño del Char incluyendo el carácter nulo
    return size;
}

void send_IO_STDIN_READ(IO_STDIN_READ* newSendIO, int socketClient) {
	t_package* package = packageCeate(READ, NEW_IO_STDIN_READ_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_STDIN_READ_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

void send_IO_STDIN_READ_WRITE_MEMORY(IO_STDIN_READ_WRITE_MEMORY* newSendIo, int socketClient) {
	t_package* package = packageCeate(IO_STDIN_READ_OP, IO_STDIN_READ_WRITE_MEMORY_StreamSize(newSendIo));
	
	t_buffer* buffer = package->buffer;

    IO_STDIN_READ_WRITE_MEMORY_Serialize(buffer, newSendIo);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}



void IO_STDIN_READ_Serialize(void* buffer, IO_STDIN_READ* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	uint32_t df = newSendIO -> physicalAdress;
    uint32_t length = newSendIO -> lengthToReadOrWrite;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &df, &offset);
	bufferAddUint32(buffer, &length, &offset);
}

void IO_STDIN_READ_WRITE_MEMORY_Serialize(void* buffer, IO_STDIN_READ_WRITE_MEMORY* newSendIo) {
	uint32_t offset = 0;

	uint32_t df = newSendIo -> physicalAdress;
    char* textToMemory = newSendIo -> textToMemory;
	uint32_t PID = newSendIo->pid;
	bufferAddUint32(buffer, &df, &offset);
	bufferAddString(buffer, textToMemory, &offset);
	bufferAddUint32(buffer, &PID, &offset);
}

IO_GEN_SLEEP_params* IO_GEN_SLEEP_Deserialize(void* stream) {

	IO_GEN_SLEEP_params* newIOsend = malloc(sizeof(IO_GEN_SLEEP_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> timeToSleep = bufferReadUint32(stream, &offset);
	printf("Read 1: %d\n", newIOsend -> pid);
	printf("Read 2: %d\n", newIOsend -> timeToSleep);

	return newIOsend;
}

IO_STDIN_READ* IO_STDIN_READ_Deserialize(void* stream) {

	IO_STDIN_READ* newIOsend = malloc(sizeof(IO_STDIN_READ));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> physicalAdress = bufferReadUint32(stream, &offset);
	newIOsend -> lengthToReadOrWrite = bufferReadUint32(stream, &offset);
	printf("Read 1: %d\n", newIOsend -> pid);
	printf("Read 2: %d\n", newIOsend -> physicalAdress);
	printf("Read 3: %d\n", newIOsend -> lengthToReadOrWrite);

	return newIOsend;
}


IO_STDIN_READ_WRITE_MEMORY* IO_STDIN_READ_WRITE_MEMORY_Deserialize(void* stream) {

	IO_STDIN_READ_WRITE_MEMORY* newIOsend = malloc(sizeof(IO_STDIN_READ_WRITE_MEMORY));
	uint32_t offset = 0;
	
	newIOsend -> physicalAdress = bufferReadUint32(stream, &offset);
	newIOsend -> textToMemory = bufferReadString(stream, &offset);
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	return newIOsend;
}


IO_STDIN_READ* recv_IO_STDIN_READ(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_STDIN_READ* IO_STDIN_READ_Recibed = IO_STDIN_READ_Deserialize(stream);
	free(stream);
	return IO_STDIN_READ_Recibed;
}

IO_STDIN_READ_WRITE_MEMORY* recv_IO_STDIN_READ_WRITE_MEMORY(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_STDIN_READ_WRITE_MEMORY* IO_STDIN_READ_WRITE_MEMORY_Recibed = IO_STDIN_READ_WRITE_MEMORY_Deserialize(stream);
	free(stream);
	return IO_STDIN_READ_WRITE_MEMORY_Recibed;
}

//-------------------------------------------STDOUT-----------------------------------------//

IO_STDOUT_WRITE* NEW_IO_STDOUT_WRITE(uint32_t pid, uint32_t DF, uint32_t length) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_STDOUT_WRITE* newIOsend = malloc(sizeof(IO_STDOUT_WRITE));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend->pid = pid;
    newIOsend->physicalAdress = DF;
	newIOsend->lengthToReadOrWrite = length;
    
    return newIOsend;
}

uint32_t NEW_IO_STDOUT_WRITE_StreamSize(IO_STDOUT_WRITE* newIOsend) {
    uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del Pid
    size += sizeof(uint32_t); // Tamaño del DF
    size += sizeof(uint32_t); // Tamaño de tamaño de lectura o escritura
    return size;
 
}

void IO_STDOUT_WRITE_Serialize(void* buffer, IO_STDOUT_WRITE* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	uint32_t df = newSendIO -> physicalAdress;
    uint32_t length = newSendIO -> lengthToReadOrWrite;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &df, &offset);
	bufferAddUint32(buffer, &length, &offset);
}



void send_IO_STDOUT_WRITE(IO_STDOUT_WRITE* newSendIO, int socketClient) {
	t_package* package = packageCeate(WRITE, NEW_IO_STDOUT_WRITE_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_STDOUT_WRITE_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
 }

 IO_STDOUT_WRITE* IO_STDOUT_WRITE_Deserialize(void* stream) {

	IO_STDOUT_WRITE* newIOsend = malloc(sizeof(IO_STDOUT_WRITE));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> physicalAdress = bufferReadUint32(stream, &offset);
	newIOsend -> lengthToReadOrWrite = bufferReadUint32(stream, &offset);

	return newIOsend;
}

IO_STDOUT_WRITE* recv_IO_STDOUT_WRITE(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_STDOUT_WRITE* IO_STDOUT_WRITE_Recibed = IO_STDOUT_WRITE_Deserialize(stream);

	return IO_STDOUT_WRITE_Recibed;

}


////////////////////////

void printFormattedText(char* text) {
    // Definir el ancho total de la línea
    const int totalWidth = 80;

    // Definir el ancho del margen izquierdo
    const int leftMargin = 10;

    // Crear las líneas de encabezado y pie de página
    char headerFooter[totalWidth + 1];
    for (int i = 0; i < totalWidth; i++) {
        headerFooter[i] = '-';
    }
    headerFooter[totalWidth] = '\0';

    // Imprimir el encabezado
    printf("%s\n", headerFooter);
    printf("%*s\n", (totalWidth + (int)strlen("Interfaz STDOUT")) / 2, "Interfaz STDOUT");
    printf("%s\n", headerFooter);

    // Imprimir el mensaje
    printf("%*s\n", (totalWidth + (int)strlen("SE IMPRIME POR PANTALLA:")) / 2, "SE IMPRIME POR PANTALLA:");
    printf("%s\n", headerFooter);

    // Tokenizar el texto por saltos de línea y centrarlas una a una
    char* line = strtok(text, "\n");
    while (line != NULL) {
        printf("%*s%s\n", leftMargin, "", line);
        line = strtok(NULL, "\n");
    }

    // Imprimir el pie de página
    printf("%s\n", headerFooter);
}


// ----------------------------------FS_CREATE---------------------------------

IO_FS_CREATE_params* NEW_IO_FS_CREATE(uint32_t pid_FS, char* fileName_FS) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_FS_CREATE_params* newIOsend_FS = malloc(sizeof(IO_FS_CREATE_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_FS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend_FS->pid = pid_FS;
    newIOsend_FS->fileName = fileName_FS;
    
    return newIOsend_FS;
}

void send_FS_CREATE (IO_FS_CREATE_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(FS_CREATE, IO_FS_CREATE_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_FS_CREATE_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t IO_FS_CREATE_StreamSize(IO_FS_CREATE_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
    size += strlen(interface -> fileName) + 1; // Tamaño del Char incluyendo el carácter nulo
    return size;
 
}

void IO_FS_CREATE_Serialize(void* buffer, IO_FS_CREATE_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	char* fileName_fs = newSendIO -> fileName;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddString(buffer, fileName_fs, &offset);
}


IO_FS_CREATE_params* recv_IO_FS_CREATE(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_FS_CREATE_params* IO_FS_CREATE_Recibed = IO_FS_CREATE_Deserialize(stream);

	free(stream);
	return IO_FS_CREATE_Recibed;

}

IO_FS_CREATE_params* IO_FS_CREATE_Deserialize(void* stream) {

	IO_FS_CREATE_params* newIOsend = malloc(sizeof(IO_FS_CREATE_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> fileName = bufferReadString(stream, &offset);

	return newIOsend;
}

// ----------------------------------FS_DELETE---------------------------------

IO_FS_DELETE_params* NEW_IO_FS_DELETE(uint32_t pid_FS, char* fileName_FS) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_FS_DELETE_params* newIOsend_FS = malloc(sizeof(IO_FS_DELETE_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_FS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend_FS->pid = pid_FS;
    newIOsend_FS->fileName = fileName_FS;
    
    return newIOsend_FS;
}

void send_FS_DELETE (IO_FS_DELETE_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(FS_DELETE, IO_FS_DELETE_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_FS_DELETE_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t IO_FS_DELETE_StreamSize(IO_FS_DELETE_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
    size += strlen(interface -> fileName) + 1; // Tamaño del Char incluyendo el carácter nulo
    return size;
 
}

void IO_FS_DELETE_Serialize(void* buffer, IO_FS_DELETE_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	char* fileName_fs = newSendIO -> fileName;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddString(buffer, fileName_fs, &offset);
}


IO_FS_DELETE_params* recv_IO_FS_DELETE(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_FS_DELETE_params* IO_FS_DELETE_Recibed = IO_FS_DELETE_Deserialize(stream);
	free(stream);
	return IO_FS_DELETE_Recibed;

}

IO_FS_DELETE_params* IO_FS_DELETE_Deserialize(void* stream) {

	IO_FS_DELETE_params* newIOsend = malloc(sizeof(IO_FS_DELETE_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> fileName = bufferReadString(stream, &offset);

	return newIOsend;
}

// ----------------------------------FS_TRUNCATE---------------------------------

IO_FS_TRUNCATE_params* NEW_IO_FS_TRUNCATE(uint32_t pid_FS, char* fileName_FS, uint32_t newLength) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_FS_TRUNCATE_params* newIOsend_FS = malloc(sizeof(IO_FS_TRUNCATE_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_FS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend_FS->pid = pid_FS;
    newIOsend_FS->fileName = fileName_FS;
	newIOsend_FS-> length = newLength;
    
    return newIOsend_FS;
}

void send_FS_TRUNCATE (IO_FS_TRUNCATE_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(FS_TRUNCATE, IO_FS_TRUNCATE_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_FS_TRUNCATE_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t IO_FS_TRUNCATE_StreamSize(IO_FS_TRUNCATE_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño del tamaño
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
    size += strlen(interface -> fileName) + 1; // Tamaño del Char incluyendo el carácter nulo
    return size;
 
}

void IO_FS_TRUNCATE_Serialize(void* buffer, IO_FS_TRUNCATE_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	char* fileName_fs = newSendIO -> fileName;
	uint32_t newLength = newSendIO -> length;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &newLength, &offset);
	bufferAddString(buffer, fileName_fs, &offset);
}


IO_FS_TRUNCATE_params* recv_IO_FS_TRUNCATE(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_FS_TRUNCATE_params* IO_FS_TRUNCATE_Recibed = IO_FS_TRUNCATE_Deserialize(stream);

	return IO_FS_TRUNCATE_Recibed;

}

IO_FS_TRUNCATE_params* IO_FS_TRUNCATE_Deserialize(void* stream) {

	IO_FS_TRUNCATE_params* newIOsend = malloc(sizeof(IO_FS_TRUNCATE_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> length = bufferReadUint32(stream, &offset);
	newIOsend -> fileName = bufferReadString(stream, &offset);

	return newIOsend;
}

// ----------------------------------FS_WRITE---------------------------------------------------

IO_FS_WRITE_params* NEW_IO_FS_WRITE(uint32_t pid_FS, char* fileName_FS, uint32_t address, uint32_t newLength, uint32_t newPointer) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_FS_WRITE_params* newIOsend_FS = malloc(sizeof(IO_FS_WRITE_params));
	newIOsend_FS->fileName = malloc(sizeof(char)*50);
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_FS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend_FS->pid = pid_FS;
    strcpy(newIOsend_FS->fileName, fileName_FS);
	newIOsend_FS->physicalAdress = address;
    newIOsend_FS->length = newLength;
	newIOsend_FS->filePointer = newPointer; 

    return newIOsend_FS;
}

void send_FS_WRITE(IO_FS_WRITE_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(FS_WRITE, IO_FS_WRITE_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_FS_WRITE_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t IO_FS_WRITE_StreamSize(IO_FS_WRITE_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño del tamaño
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
    size += strlen(interface -> fileName) + 1; // Tamaño del Char incluyendo el carácter nulo
	size += sizeof(uint32_t); // Tamaño de la direccion
	size += sizeof(uint32_t); // Tamaño del puntero
    return size;
 
}

void IO_FS_WRITE_Serialize(void* buffer, IO_FS_WRITE_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	char* fileName_fs = newSendIO -> fileName;
	uint32_t newAddress = newSendIO->physicalAdress;
	uint32_t newLength = newSendIO -> length;
	uint32_t trueFilePointer = newSendIO->filePointer;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddString(buffer, fileName_fs, &offset);
	bufferAddUint32(buffer, &newAddress, &offset);
	bufferAddUint32(buffer, &newLength, &offset);
	bufferAddUint32(buffer, &trueFilePointer, &offset);
}


IO_FS_WRITE_params* recv_IO_FS_WRITE(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_FS_WRITE_params* IO_FS_WRITE_Recibed = IO_FS_WRITE_Deserialize(stream);

	return IO_FS_WRITE_Recibed;

}


IO_FS_WRITE_params* IO_FS_WRITE_Deserialize(void* stream) {

	IO_FS_WRITE_params* newIOsend = malloc(sizeof(IO_FS_WRITE_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> fileName = bufferReadString(stream, &offset);
	newIOsend -> physicalAdress = bufferReadUint32(stream, &offset);
	newIOsend -> length = bufferReadUint32(stream, &offset);
	newIOsend -> filePointer = bufferReadUint32(stream, &offset);

	return newIOsend;
}

// ----------------------------------FS_READ--------------------------------------------------

IO_FS_READ_params* NEW_IO_FS_READ(uint32_t pid_FS, char* fileName_FS, uint32_t address, uint32_t newLength, uint32_t newPointer) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_FS_READ_params* newIOsend_FS = malloc(sizeof(IO_FS_READ_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_FS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend_FS->pid = pid_FS;
    newIOsend_FS->fileName = fileName_FS;
	newIOsend_FS->physicalAdress = address;
    newIOsend_FS->length = newLength;
	newIOsend_FS->filePointer = newPointer; 

    return newIOsend_FS;
}

void send_FS_READ(IO_FS_WRITE_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(FS_READ, IO_FS_READ_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_FS_READ_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t IO_FS_READ_StreamSize(IO_FS_READ_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño del tamaño
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
    size += strlen(interface -> fileName) + 1; // Tamaño del Char incluyendo el carácter nulo
	size += sizeof(uint32_t); // Tamaño de la direccion
	size += sizeof(uint32_t); // Tamaño del puntero
    return size;
 
}

void IO_FS_READ_Serialize(void* buffer, IO_FS_READ_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO -> pid;
	char* fileName_fs = newSendIO -> fileName;
	uint32_t newAddress = newSendIO->physicalAdress;
	uint32_t newLength = newSendIO -> length;
	uint32_t trueFilePointer = newSendIO->filePointer;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddString(buffer, fileName_fs, &offset);
	bufferAddUint32(buffer, &newAddress, &offset);
	bufferAddUint32(buffer, &newLength, &offset);
	bufferAddUint32(buffer, &trueFilePointer, &offset);
}


IO_FS_READ_params* recv_IO_FS_READ(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_FS_READ_params* IO_FS_READ_Recibed = IO_FS_READ_Deserialize(stream);

	return IO_FS_READ_Recibed;

}


IO_FS_READ_params* IO_FS_READ_Deserialize(void* stream) {

	IO_FS_READ_params* newIOsend = malloc(sizeof(IO_FS_READ_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> fileName = bufferReadString(stream, &offset);
	newIOsend -> physicalAdress = bufferReadUint32(stream, &offset);
	newIOsend -> length = bufferReadUint32(stream, &offset);
	newIOsend -> filePointer = bufferReadUint32(stream, &offset);

	return newIOsend;
}

//------------------------------------------------------------------
IO_FS_READ_TO_MEMORY_params* NEW_IO_FS_READ_TO_MEMORY(uint32_t pid, uint32_t length, uint32_t address, char* newWord) {
    // Reserva memoria para una nueva estructura io_write_or_read
    IO_FS_READ_TO_MEMORY_params* newIOsend_FS = malloc(sizeof(IO_FS_READ_TO_MEMORY_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_FS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
	newIOsend_FS-> pid = pid;
	newIOsend_FS-> length = length;
	newIOsend_FS-> word = newWord;
    newIOsend_FS-> physicalAdress = address;

    return newIOsend_FS;
}

void send_FS_READ_TO_MEMORY(IO_FS_READ_TO_MEMORY_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(FS_READ, IO_FS_READ_TO_MEMORY_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    IO_FS_READ_TO_MEMORY_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t IO_FS_READ_TO_MEMORY_StreamSize(IO_FS_READ_TO_MEMORY_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t);
	size += sizeof(uint32_t);
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
    size += strlen(interface -> word) + 1; // Tamaño del Char incluyendo el carácter nulo

    return size;
 
}

void IO_FS_READ_TO_MEMORY_Serialize(void* buffer, IO_FS_READ_TO_MEMORY_params* newSendIO) {
	uint32_t offset = 0;

	uint32_t pid = newSendIO->pid;
	uint32_t length = newSendIO->length;
	uint32_t newAddress = newSendIO->physicalAdress;
	char* wordToSend = newSendIO->word;
	
	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &length, &offset);
	bufferAddUint32(buffer, &newAddress, &offset);
	bufferAddString(buffer, wordToSend, &offset);

}


IO_FS_READ_TO_MEMORY_params* recv_IO_FS_READ_TO_MEMORY_(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	IO_FS_READ_TO_MEMORY_params* IO_FS_READ_TO_MEMORY_Recibed = IO_FS_READ_TO_MEMORY_Deserialize(stream);

	return IO_FS_READ_TO_MEMORY_Recibed;

}


IO_FS_READ_TO_MEMORY_params* IO_FS_READ_TO_MEMORY_Deserialize(void* stream) {

	IO_FS_READ_TO_MEMORY_params* newIOsend = malloc(sizeof(IO_FS_READ_TO_MEMORY_params));
	uint32_t offset = 0;

	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend -> length = bufferReadUint32(stream, &offset);
	newIOsend -> physicalAdress = bufferReadUint32(stream, &offset);
	newIOsend -> word = bufferReadString(stream, &offset);

	return newIOsend;
}

