#include"utils.h"

//t_log* logger;
t_config* config;

// int waitClient(int socketServer)
// {
// 	// Aceptamos un nuevo cliente
// 	int socketClient= accept(socketServer, NULL, NULL);

// 	log_info(logger, "A client connected");
// 	size_t bytes;
// 	int32_t handshake;
// 	int32_t resultOk = 0;
// 	int32_t resultError = -1;
	
// 	bytes = recv(socketClient, &handshake, sizeof(int32_t), MSG_WAITALL);

// 	if (handshake == 1) {
// 		bytes = send(socketClient, &resultOk, sizeof(int32_t), 0);
// 	} else {
// 		bytes = send(socketClient, &resultError, sizeof(int32_t), 0);
// 	}
	
// 	return socketClient;
// }

char* moduleChar(MODULE module) {
	char* moduleChar = malloc(sizeof(char)*15);
	switch (module)
	{
	case KERNEL:
		strcpy(moduleChar, "Kernel");
	break;
	case CPU:
		strcpy(moduleChar, "CPU");
	break;
	case MEMORY:
		strcpy(moduleChar, "Memory");
	break;
	case IO:
		strcpy(moduleChar, "IO");
	break;

	default:
		break;
	}
	return moduleChar;
}

int recvOperation(int socketClient)
{
	int cod_op;
	if(recv(socketClient, &cod_op, sizeof(int32_t), MSG_WAITALL) > 0) {
		return cod_op;
	}
		
	else
	{
		close(socketClient);
		return -1;
	}
}
void* recvBufferInstruction(int* size, int socketClient) {
    int32_t messageSize;

    // Recibir el tamaño del mensaje
    if (recv(socketClient, &messageSize, sizeof(int32_t), MSG_WAITALL) <= 0) {
        log_error(logger, "Error al recibir el tamaño del mensaje");
        return NULL;
    }

    // Reservar memoria para el buffer
    void* buffer = malloc(messageSize);
    if (buffer == NULL) {
        log_error(logger, "Error al asignar memoria para el buffer");
        return NULL;
    }

    // Recibir el contenido del mensaje
    if (recv(socketClient, buffer, messageSize, MSG_WAITALL) <= 0) {
        log_error(logger, "Error al recibir el mensaje");
        free(buffer); // Liberar la memoria asignada para el buffer
        return NULL;
    }

    // Asignar el tamaño del mensaje recibido
    *size = messageSize;

    printf("Mensaje recibido correctamente\n");

    return buffer;
}


char* recvInstruction(int socketClient) {
    int size;
    char* buffer = (char*)recvBufferInstruction(&size, socketClient);
    if (buffer == NULL) {
        return NULL;
    }

    int32_t result = 0;
    if (send(socketClient, &result, sizeof(int32_t), 0) == -1) {
        log_error(logger, "Error al enviar la confirmación de recepción de instrucción");
    }
	printf("[char* recvInstruction] Dirección de memoria de buffer: %p\n", (void*)buffer);
    return buffer;
}


void* recvBuffer(int* size, int socketClient)
{
	void * buffer;

	recv(socketClient, size, sizeof(int), MSG_WAITALL);

	buffer = malloc(*size);

	recv(socketClient, buffer, *size, MSG_WAITALL);


	return buffer;
}

char* recvMessage(int socketClient)
{
	int size;
	char* buffer = recvBuffer(&size, socketClient);
	int32_t result = 0;
	send(socketClient, &result, sizeof(int32_t), 0);
	log_info(logger, "Message recived: %s", buffer);
	return buffer;
}

char* recvSTDOUT(int socketClient)
{
	int size;
	char* buffer = recvBuffer(&size, socketClient);
	int32_t result = 0;
	log_info(logger, "Message recived: %s", buffer);
	return buffer;
}

int sendInt(int value, int socket) {
    int bytes_enviados = send(socket, &value, sizeof(int), 0);
    if (bytes_enviados != sizeof(int)) {
        // Error al enviar el entero
        return -1; // Código de error personalizable
    }

    // Todo bien
    return 0;
}



int recvInt(int* value, int socket) {
    int bytes_leidos = read(socket, value, sizeof(int));
    if (bytes_leidos != sizeof(int)) {
        // Error al leer el entero
        return -1; // Código de error personalizable
    }

    // Todo ha ido bien
    return 0;
}

t_list* recvPack(int socketClient)
{
	int size;
	int displacement = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recvBuffer(&size, socketClient);
	while(displacement < size)
	{
		memcpy(&tamanio, buffer + displacement, sizeof(int));
		displacement+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+displacement, tamanio);
		displacement+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

char* getConfigValue(t_config* config, char* key) {

	char* value;

	if( config_has_property(config, key) ) {

		value = config_get_string_value(config, key);
		//log_info(logger, "The %s field has the value: %s. \n", key, value);
		return value;
	} else {

		//log_info(logger, "Error. %s field not found. \n", key);
		exit(EXIT_FAILURE);
	}
}

t_log* initLogger(char* file, char* process_name, bool is_active_console, t_log_level level) {

	t_log* newLogger;

	newLogger = log_create(file, process_name, is_active_console, level);

	return newLogger;
}

t_config* initConfig(char* configPath) {
	t_config* newConfig;
	
	newConfig = config_create(configPath);
	return newConfig;
}

void finishProgram(int conection, t_log* logger, t_config* config) {
	
	if( logger != NULL ) {
		log_destroy(logger);
	}

	if ( config != NULL) {
		config_destroy(config);
	}

	if (conection != NULL) {
		closeConection(conection);
	}
}

void closeConection(int socketClient) {
	close(socketClient);
}

void* serializePackage(t_package* package, int bytes)
{
	void * magic = malloc(bytes);
	int displacement = 0;

	memcpy(magic + displacement, &(package->operationCode), sizeof(int));
	displacement+= sizeof(int);
	memcpy(magic + displacement, &(package->buffer->size), sizeof(int));
	displacement+= sizeof(int);
	memcpy(magic + displacement, package->buffer->stream, package->buffer->size);
	displacement+= package->buffer->size;

	return magic;
}

t_package* createPackage(void) {

	t_package* package = malloc(sizeof(t_package));
	package->operationCode = PACKAGE;
	newBuffer(package);
	return package;
}

void addPackage(t_package* package, void* valor, int extraSize) {

	package->buffer->stream = realloc(package->buffer->stream, package->buffer->size + extraSize + sizeof(int));

	memcpy(package->buffer->stream + package->buffer->size, &extraSize, sizeof(int));
	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), valor, extraSize);

	package->buffer->size += extraSize + sizeof(int);
}

void sendPackage(t_package* package, int socketClient) {

	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
}

void deletePackage(t_package* package)
{
	free(package->buffer->stream);
	free(package->buffer);
	free(package);
}

void sendMessage(char* message, int socketClient) {

	t_package* package = malloc(sizeof(t_package));

	package->operationCode = MESSAGE;
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

void sendResourceWAIT(char* message, int socketClient) {

	t_package* package = malloc(sizeof(t_package));

	package->operationCode = WAIT;
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

void sendResourceSIGNAL(char* message, int socketClient) {

	t_package* package = malloc(sizeof(t_package));

	package->operationCode = SIGNAL;
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



char* recvResourceWait(int socketClient)
{
	int size;
	char* buffer = recvBuffer(&size, socketClient);
	int32_t result = 0;
	//send(socketClient, &result, sizeof(int32_t), 0);
	log_info(logger, "Resource for Wait recived: %s", buffer);
	return buffer;
}

char* recvResourceSignal(int socketClient)
{
	int size;
	char* buffer = recvBuffer(&size, socketClient);
	int32_t result = 0;
	//send(socketClient, &result, sizeof(int32_t), 0);
	log_info(logger, "Resource for Signal recived: %s", buffer);
	return buffer;
}


// Función para buscar un valor en un array de cadenas y devolver su índice
int findIndexOfString(char** array, size_t size, const char* value) {
    for (size_t i = 0; i < size; i++) {
        if (strcmp(array[i], value) == 0) {
            return (int)i;
        }
    }
    return -1; // Valor no encontrado
}


void sendInterrupt(char* message, int socketClient) {

	t_package* package = malloc(sizeof(t_package));

	package->operationCode = INTERRUPTBYQUANTUM;
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

void sendLine(const char* message, int socketClient) {
    int32_t size = strlen(message) + 1; // Incluye el carácter nulo

    // Enviar el tamaño del mensaje
    if (send(socketClient, &size, sizeof(int32_t), 0) == -1) {
        log_error(logger, "Error al enviar el tamaño del mensaje");
        return;
    }

    // Enviar el mensaje
    if (send(socketClient, message, size, 0) == -1) {
        log_error(logger, "Error al enviar el mensaje");
        return;
    }
}

void newBuffer(t_package* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}


char* detectModuleById(MODULE module)
{
	char* aux = malloc(sizeof(char)*50);
    switch(module) {
        case KERNEL:
			strcpy(aux, "KERNEL");
			break;
        case CPU:
            strcpy(aux, "CPU");
			break;
        case MEMORY:
            strcpy(aux, "MEMORY");
			break;
        case IO:
            strcpy(aux, "IO");
			break;
        default:
            strcpy(aux, "VALOR NO RECONOCIDO");
			break;
    }
	return aux;
}

const char* module_strings[] = {
    "KERNEL",
    "CPU",
    "MEMORY",
    "IO",
};



void send_WAIT_OR_SIGNAL(uint32_t result, int socketClient) {

	t_package* package = packageCeate(result, send_WAIT_OR_SIGNAL_StreamSize());
	
	t_buffer* buffer = package->buffer;

    send_WAIT_OR_SIGNAL_Serialize(buffer, result);

	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}


uint32_t recv_WAIT_OR_SIGNAL(int socketClient)
{
	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	uint32_t WAIT_OR_SIGNAL = WAIT_OR_SIGNAL_Deserialize(stream);
	free(stream);
	return WAIT_OR_SIGNAL;
}



uint32_t send_WAIT_OR_SIGNAL_StreamSize() {
    uint32_t size = 0;
    size += sizeof(uint32_t);
    return size;
 
}


void send_WAIT_OR_SIGNAL_Serialize(void* buffer, uint32_t value) {
	uint32_t offset = 0;

	uint32_t number = value;
	bufferAddUint32(buffer, &value, &offset);
}

uint32_t WAIT_OR_SIGNAL_Deserialize(void* stream) {

	uint32_t offset = 0;
	
	uint32_t WAIT_OR_SIGNAL = bufferReadUint32(stream, &offset);
	printf("Read 1: %d\n", WAIT_OR_SIGNAL);

	return WAIT_OR_SIGNAL;
}







MODULE detectModuleByName(const char* module_name) {
    for (int i = 0; i < sizeof(module_strings) / sizeof(module_strings[0]); i++) {
        if (strcmp(module_name, module_strings[i]) == 0) {
            return (MODULE)(i + 1);  // Ajustar el índice para que coincida con los valores del enum
        }
    }
	log_warning(logger, "La interfaz no se reconoce por el SO");
    return -1 ; // Si no hay coincidencia, devuelve UNKNOWN_MODULE
}







void sendPcb(pcb* pcb, int socketClient) {
    t_package* package = packageCeate(PCB, pcbStreamSize(pcb));
    t_buffer* buffer = package->buffer;
	printf("[void sendPCB] Dirección de memoria de buffer: %p\n", (void*)buffer);
    pcbSerialize(buffer, pcb);

    int bytes = package->buffer->size + 2 * sizeof(int);
    void* toSend = serializePackage(package, bytes);

    // Logging para verificar el contenido y tamaño del buffer
    printf("Sending PCB with size: %d\n", package->buffer->size);

    send(socketClient, toSend, bytes, MSG_NOSIGNAL);

    free(toSend);
    packageDestroy(package);
}

void sendProcessToMemory(processToMemory* newProcess, int socketClient) {
	t_package* package = packageCeate(PROCESSTOMEMORY,processToMemoryStreamSize(newProcess));
	
	t_buffer* buffer = package->buffer;

    processToMemorySerialize(buffer, newProcess);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);//ejemplo para MMU

	free(toSend);
	packageDestroy(package);
}


pcb* recvPcb(int socketClient) {
	//pcb* pcbRecibed = malloc(sizeof(pcb));
	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	pcb* pcbRecibed = pcbDeserialize(stream);

	return pcbRecibed;
}

void pcbSerialize(void* buffer, pcb* pcb) {
	uint32_t offset = 0;

	uint32_t pid = pcb->pid;
	uint32_t quantum = pcb->quantum;
    CpuRegister registers = pcb->cpuRegisters;
    stateProcess state = pcb->state;
	contextSwitch* context = pcb->context;

	bufferAddUint32(buffer, &pcb->pid, &offset);
    bufferAddUint32(buffer, &pcb->quantum, &offset);
    bufferAddCpuRegister(buffer, &pcb->cpuRegisters, &offset);
    bufferAddUint32(buffer, &pcb->state, &offset);
    bufferAddCcontextSwitch(buffer, pcb->context, &offset);
}

void printBuffer(void* buffer, uint32_t size) {
    uint8_t* byteBuffer = (uint8_t*)buffer;
    for (uint32_t i = 0; i < size; ++i) {
        printf("%02x ", byteBuffer[i]);
    }
    printf("\n");
}

pcb* pcbDeserialize(void* stream) {
	pcb* newPcb = malloc(sizeof(pcb));
	uint32_t offset = 0;
	
	// Prueba para leer el buffer, simulando lo que haria la cpu para recibir
	newPcb->pid = bufferReadUint32(stream, &offset);
	newPcb->quantum = bufferReadUint32(stream, &offset);
	newPcb->cpuRegisters = bufferReadCpuRegister(stream, &offset);
	newPcb->state = (stateProcess)bufferReadUint32(stream, &offset);
	newPcb->context = bufferReadcontextSwitch(stream, &offset);
	/* printf("Read pid: %u\n", newPcb->pid);
	printf("Read quantum: %u\n", newPcb->quantum);
	printf("Read PC: %u\n", *newPcb->cpuRegisters.PC);
	printf("Read AX: %u\n", *newPcb->cpuRegisters.AX);
	printf("Read BX: %u\n", *newPcb->cpuRegisters.BX);
	printf("Read CX: %u\n", *newPcb->cpuRegisters.CX);
	printf("Read DX: %u\n", *newPcb->cpuRegisters.DX);
	printf("Read EAX: %u\n", *newPcb->cpuRegisters.EAX);
	printf("Read EBX: %u\n", *newPcb->cpuRegisters.EBX);
	printf("Read ECX: %u\n", *newPcb->cpuRegisters.ECX);
	printf("Read EDX: %u\n", *newPcb->cpuRegisters.EDX);
	printf("Read SI: %u\n", *newPcb->cpuRegisters.SI);
	printf("Read DI: %u\n", *newPcb->cpuRegisters.DI);
	printf("Read Context Reason: %s\n", newPcb->context->reason);
	printf("Read Context IO: %s\n", newPcb->context->io); */
	free(stream);
	return newPcb;

}

void processToMemorySerialize(void* buffer, processToMemory* newProcess) {
	uint32_t offset = 0;

	uint32_t pid = newProcess -> pid;
	uint32_t tamanio = newProcess -> tamanioProceso;
    char* path = newProcess -> path;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &tamanio, &offset);
	bufferAddString(buffer, path, &offset);
}

processToMemory* memoryToProcessDeserialize(void* stream) {

	processToMemory* newProcess = malloc(sizeof(processToMemory));
	uint32_t offset = 0;
	
	// Prueba para leer el buffer, simulando lo que haria la cpu para recibir
	newProcess -> pid = bufferReadUint32(stream, &offset);
	newProcess -> tamanioProceso = bufferReadUint32(stream, &offset);
	newProcess -> path = bufferReadString(stream, &offset);
	printf("Read 1: %u\n", newProcess -> pid);
	printf("Read 2: %d\n", newProcess -> tamanioProceso);
	printf("Read 3: %s\n", newProcess -> path);

	return newProcess;
}

instReqToMemory instReqToMemoryDeserialize (void* stream) {
	instReqToMemory newReq;
	uint32_t offset = 0;
	
	// Prueba para leer el buffer, simulando lo que haria la cpu para recibir
	newReq.pid = bufferReadUint32(stream, &offset);
	newReq.programCounter = bufferReadUint32(stream, &offset);

	return newReq;
}

processToMemory* recvProcessToMemory(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	processToMemory* processToMemoryRecibed = memoryToProcessDeserialize(stream);

	free(stream);  // VER SI SOLUCIONA MEMORY LEAK 130 BYTES (memoria)

	return processToMemoryRecibed;
}

instReqToMemory recvInstReqToMemory(int socketClient) {
	int size;
	void * stream;
	stream = recvBuffer(&size, socketClient);
	instReqToMemory request = instReqToMemoryDeserialize(stream);

	free(stream);  // VER SI SOLUCIONA MEMORY LEAK 8 BYTES (memoria)

	return request;
}

int32_t recvCpuPid(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);

	int32_t cpuPidRecived = cpuPidAndCodMemoryDeserialize(stream);

	return cpuPidRecived ;
}


int32_t cpuPidAndCodMemoryDeserialize(void* stream) {

	int32_t pid  = malloc(sizeof(int32_t));
	uint32_t offset = 0;
	
	// Prueba para leer el buffer, simulando lo que haria la cpu para recibir
 	pid = bufferReadUint32(stream, &offset);
	printf("Read 1: %u\n", pid );
	return pid;
}
uint32_t cpuPidAndCodStreamSize(int32_t request) {
    uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del pid
    return size;
}
void cpuPidAndCodSerialize(void* buffer, int32_t request) {
	uint32_t offset = 0;

	uint32_t pid = request;

	bufferAddUint32(buffer, &pid, &offset);
}

void instReqToMemorySerialize(void* buffer, instReqToMemory request) {
	uint32_t offset = 0;
	uint32_t pid = request.pid;
	uint32_t programCounter = request.programCounter;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &programCounter, &offset);
}


solicitudFrameMemoria* NUEVA_SOLICITUD_FRAME_MEMORIA(uint32_t PID, int pagina) {
    // Reserva memoria para una nueva estructura io_write_or_read
    solicitudFrameMemoria* newRequestFRAME = malloc(sizeof(solicitudFrameMemoria));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newRequestFRAME == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
	newRequestFRAME -> pid = PID;
    newRequestFRAME -> paginas = pagina;

    return newRequestFRAME ;
}

void send_SOLICITUD_FRAME_MEMORIA(solicitudFrameMemoria* newRequestFRAME, int socketClient) {

	t_package* package = packageCeate(SOLICITUD_FRAME, SOLICITUD_FRAME_MEMORIA_StreamSize(newRequestFRAME));
	
	t_buffer* buffer = package->buffer;

    SOLICITUD_FRAME_MEMORIA_Serialize(buffer, newRequestFRAME);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t SOLICITUD_FRAME_MEMORIA_StreamSize(solicitudFrameMemoria* newRequestFRAME) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(int); // Tamaño de pagina

    return size;
 
}

void SOLICITUD_FRAME_MEMORIA_Serialize(void* buffer, solicitudFrameMemoria* newRequestFRAME) {

	uint32_t offset = 0;

	uint32_t pid = newRequestFRAME->pid;
	uint32_t pagina = newRequestFRAME->paginas;
	

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &pagina, &offset);

}


solicitudFrameMemoria* recv_SOLICITUD_FRAME_MEMORIA(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	solicitudFrameMemoria* SOLICITUD_FRAME_MEMORIA_Recibed = SOLICITUD_FRAME_MEMORIA_Deserialize(stream);

	free(stream);  // VER SI SOLUCIONA MEMORY LEAK

	return SOLICITUD_FRAME_MEMORIA_Recibed;

}


solicitudFrameMemoria* SOLICITUD_FRAME_MEMORIA_Deserialize(void* stream) {

	solicitudFrameMemoria* newRequestFRAME = malloc(sizeof(solicitudFrameMemoria));
	uint32_t offset = 0;
	

	newRequestFRAME -> pid = bufferReadUint32(stream, &offset);
	newRequestFRAME -> paginas = bufferReadUint32(stream, &offset);


	return newRequestFRAME;
}




char* intToChar(int num) {
    char* str = (char*)malloc(20 * sizeof(char));
    sprintf(str, "%d", num);
    return str;
}

t_queue* splitString(char* str, const char* delimiter) {
    // Creamos un array de punteros de char para almacenar las palabras
	t_queue* words = queue_create();
    if (words == NULL) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }

    bool inQuotes = false;
    char* start = str;
    char* token = NULL;
    size_t length = strlen(str);

	printf("SEPARAMOS: %s\n", str);

    for (size_t i = 0; i <= length; i++) {
        char currentChar = str[i];
        if (currentChar == '"' && (i == 0 || str[i - 1] != '\\')) {  // Detectamos comillas no escapadas
            inQuotes = !inQuotes;
            if (inQuotes) {
                start = &str[i + 1];  // Saltar la comilla de inicio
            } else {
                str[i] = '\0';  // Terminar el token sin incluir la comilla de cierre
                token = start;
                start = &str[i + 1];  // Mover el inicio al carácter después de la comilla de cierre
            }
        } else if (!inQuotes && (strchr(delimiter, currentChar) != NULL || currentChar == '\0' || currentChar == '\n')) {
            if (currentChar != '\0') {
                str[i] = '\0';
            }
            if (start != &str[i]) {
                token = start;
            }
            start = &str[i + 1];
        }

        if (token != NULL) {
            // Asegurarse de que no se incluyan comillas en el token
            char* newWord = (char*)malloc((strlen(token) + 1) * sizeof(char));
            if (newWord == NULL) {
                printf("Error: No se pudo asignar memoria.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(newWord, token);
            queue_push(words, newWord);
            token = NULL;
        }

        if (currentChar == '\0') {
            break;
        }
    }

    if (inQuotes) {
        printf("Error: Comillas desbalanceadas.\n");
        return NULL;
    }

    return words;
}
char* getNextWord(char* operation, int* offset){
    char* buffer = malloc(sizeof(char)*50);
    int i = 0;
        while (operation[(*offset)] != '\0' && operation[(*offset)] != ' ' && operation[(*offset)] != '\n'){   // Lee el la palabra hasta el espacio o fin de cadena
        char caracter = operation[(*offset)];      // Copia caractér por caracter lo leido
        buffer[i] = operation[(*offset)];  // Pega caractér copiado en el buffer
        (*offset)++;        // Mueve la posicion de la siguiente lectura                                                                                               // Mueve la posicion de la siguiente lectura                                                            // Mueve la posicion de la siguiente lectura
        i++;
    }
    buffer[i] = '\0';
    (*offset)++;     // Deja fuera al espacio o fin de cadena 
    return buffer;
}

// int getValue(char* buffer){         // Recibe char* con numeros y lo devuelve como int, deveria valer tambien para las instrucciones que asumo son int por los ejemplos 

//     int value = 0;

//     if(isdigit(*buffer)) {
//         value = atoi(buffer);
//     } else {
//         printf("EL buffer: %s - No es un digito\n", buffer) ;     // Comportamiento ante el caso erroneo en el que buffer no contenga un numero
//     }
//     return value;    
// }


int getValue(char* buffer) {
    if (buffer == NULL || *buffer == '\0') {
        printf("El buffer está vacío o es nulo\n");
        return 0;
    }
    
    if (*buffer == '-' || isdigit(*buffer)) {
        char *endptr;
        int value = strtol(buffer, &endptr, 10);
        
        if (*endptr == '\0') {
            return value;
        } else {
            printf("El buffer: %s - No es un número válido\n", buffer);
            return 0;
        }
    } else {
        printf("El buffer: %s - No es un dígito ni un signo negativo\n", buffer);
        return 0;
    }
}

void print_rectangle(const char* message) {
    int message_length = strlen(message);
    int padding = 4;  // Padding on each side of the message
    int width = message_length + padding * 2;
    int height = 5;

    // Print top border
    for (int i = 0; i < width; i++) {
        printf("*");
    }
    printf("\n");

    // Print empty lines
    for (int i = 1; i < height - 1; i++) {
        printf("*");
        for (int j = 1; j < width - 1; j++) {
            if (i == height / 2 && j == (width - message_length) / 2) {
                printf("%s", message);
                j += message_length - 1;  // Move the index to the end of the message
            } else {
                printf(" ");
            }
        }
        printf("*\n");
    }

    // Print bottom border
    for (int i = 0; i < width; i++) {
        printf("*");
    }
    printf("\n");
}

RESIZE_params* NEW_RESIZE(int pid, int nuevoTamanioProceso) {
    // Reserva memoria para una nueva estructura io_write_or_read
    RESIZE_params* newResize = malloc(sizeof(RESIZE_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newResize == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newResize -> pid = pid;
	newResize -> tamanioProceso = nuevoTamanioProceso;
    
    return newResize;
}

void send_RESIZE (RESIZE_params* tamanioProceso, int socketClient) {

	t_package* package = packageCeate(RESIZE, RESIZE_StreamSize(tamanioProceso));
	
	t_buffer* buffer = package->buffer;

    RESIZE_Serialize(buffer, tamanioProceso);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t RESIZE_StreamSize(RESIZE_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño del tamaño
    return size;
 
}

void RESIZE_Serialize(void* buffer, RESIZE_params* nuevoTamanioProceso) {
	uint32_t offset = 0;

	uint32_t pid = nuevoTamanioProceso -> pid;
	uint32_t newLength = nuevoTamanioProceso->tamanioProceso;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &newLength , &offset);
}


RESIZE_params* recv_RESIZE(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	RESIZE_params* RESIZE_Recibed = RESIZE_Deserialize(stream);

	free(stream);  //  VER SI SOLUCIONA MEMORY LEAK 16 BYTES

	return RESIZE_Recibed;

}

RESIZE_params* RESIZE_Deserialize(void* stream) {

	RESIZE_params* nuevoTamanioProceso = malloc(sizeof(RESIZE_params));
	uint32_t offset = 0;
	
	nuevoTamanioProceso -> pid = bufferReadUint32(stream, &offset);
	nuevoTamanioProceso -> tamanioProceso = bufferReadUint32(stream, &offset);

	return nuevoTamanioProceso;
}

// Función para eliminar espacios en blanco al principio y al final de una cadena
char* trim(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}
// Dividir la cadena y almacenar en el array de cadenas
void parseStringToStringArray(char* str, char*** strArray, size_t* arraySize) {
    // Eliminar los corchetes
    str++;
    str[strlen(str) - 1] = '\0';

    // Contar el número de elementos
    size_t count = 1;
    for (char* temp = str; *temp; temp++) {
        if (*temp == ',') {
            count++;
        }
    }

    // Crear el array de cadenas
    *strArray = malloc(count * sizeof(char*));
    if (*strArray == NULL) {
        perror("Unable to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Dividir la cadena y almacenar en el array de cadenas
    size_t index = 0;
    char* token = strtok(str, ",");
    while (token != NULL) {
        token = trim(token); // Eliminar espacios en blanco
        (*strArray)[index] = malloc(strlen(token) + 1);
        if ((*strArray)[index] == NULL) {
            perror("Unable to allocate memory");
            exit(EXIT_FAILURE);
        }
        strcpy((*strArray)[index], token);
        index++;
        token = strtok(NULL, ",");
    }

    *arraySize = count;
}

// Función para eliminar los corchetes de una cadena y convertirla directamente en un array de enteros
void parseStringToIntArray(char* str, int** intArray, size_t* arraySize) {
    // Eliminar los corchetes
    str++;
    str[strlen(str) - 1] = '\0';

    // Contar el número de elementos
    size_t count = 1;
    for (char* temp = str; *temp; temp++) {
        if (*temp == ',') {
            count++;
        }
    }

    // Crear el array de enteros
    *intArray = malloc(count * sizeof(int));
    if (*intArray == NULL) {
        perror("Unable to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Dividir la cadena y convertir a enteros
    size_t index = 0;
    char* token = strtok(str, ",");
    while (token != NULL) {
        token = trim(token); // Eliminar espacios en blanco
        (*intArray)[index++] = atoi(token);
        token = strtok(NULL, ",");
    }

    *arraySize = count;
}


MOVE_IN_TO_MEMORY_params* NEW_MOVE_IN_TO_MEMORY(uint32_t pidMOVE_IN_TO_MEMORY, int length, uint32_t address) {
    // Reserva memoria para una nueva estructura io_write_or_read
    MOVE_IN_TO_MEMORY_params* newMOVE_IN_TO_MEMORY = malloc(sizeof(MOVE_IN_TO_MEMORY_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newMOVE_IN_TO_MEMORY == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    newMOVE_IN_TO_MEMORY -> pid = pidMOVE_IN_TO_MEMORY;
	newMOVE_IN_TO_MEMORY -> physicalAdress = address;
    newMOVE_IN_TO_MEMORY -> length = length;

    return newMOVE_IN_TO_MEMORY;
}

void send_MOVE_IN_TO_MEMORY(MOVE_IN_TO_MEMORY_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(MOVE_IN_OP, MOVE_IN_TO_MEMORY_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    MOVE_IN_TO_MEMORY_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t MOVE_IN_TO_MEMORY_StreamSize(MOVE_IN_TO_MEMORY_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño del tamaño
	size += sizeof(uint32_t); // Tamaño de la direccion
    return size;
 
}

void MOVE_IN_TO_MEMORY_Serialize(void* buffer, MOVE_IN_TO_MEMORY_params* newSend) {
	uint32_t offset = 0;

	uint32_t pid = newSend -> pid;
	uint32_t newAddress = newSend->physicalAdress;
	uint32_t newLength = newSend -> length;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &newAddress, &offset);
	bufferAddUint32(buffer, &newLength, &offset);
}


MOVE_IN_TO_MEMORY_params* recv_MOVE_IN_TO_MEMORY(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	MOVE_IN_TO_MEMORY_params* MOVE_IN_TO_MEMORY_Recibed = MOVE_IN_TO_MEMORY_Deserialize(stream);

	free(stream);  // VER SI SOLUCIONA MEMORY LEAK 24 BYTES

	return MOVE_IN_TO_MEMORY_Recibed;

}


MOVE_OUT_TO_MEMORY_params* MOVE_IN_TO_MEMORY_Deserialize(void* stream) {

	MOVE_OUT_TO_MEMORY_params* new = malloc(sizeof(MOVE_OUT_TO_MEMORY_params));
	uint32_t offset = 0;
	
	new -> pid = bufferReadUint32(stream, &offset);
	new-> physicalAdress = bufferReadUint32(stream, &offset);
	new -> length = bufferReadUint32(stream, &offset);

	return new;
}

MOVE_OUT_TO_MEMORY_params* NEW_MOVE_OUT_TO_MEMORY(uint32_t pidMOVE_OUT_TO_MEMORY, int tamanioParaLeer, uint32_t address, uint32_t dataRegistro) {
    // Reserva memoria para una nueva estructura io_write_or_read
    MOVE_OUT_TO_MEMORY_params* newMOVE_OUT_TO_MEMORY = malloc(sizeof(MOVE_OUT_TO_MEMORY_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newMOVE_OUT_TO_MEMORY == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    newMOVE_OUT_TO_MEMORY -> pid = pidMOVE_OUT_TO_MEMORY;
	newMOVE_OUT_TO_MEMORY->length = tamanioParaLeer;
	newMOVE_OUT_TO_MEMORY -> physicalAdress = address;
	newMOVE_OUT_TO_MEMORY -> dataRegistro = dataRegistro;

    return newMOVE_OUT_TO_MEMORY;
}

void send_MOVE_OUT_TO_MEMORY(MOVE_OUT_TO_MEMORY_params* newSendIO, int socketClient) {

	t_package* package = packageCeate(MOVE_OUT_OP, MOVE_OUT_TO_MEMORY_StreamSize(newSendIO));
	
	t_buffer* buffer = package->buffer;

    MOVE_OUT_TO_MEMORY_Serialize(buffer, newSendIO);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t MOVE_OUT_TO_MEMORY_StreamSize(MOVE_OUT_TO_MEMORY_params* interface) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(int); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño de la direccion
	size += sizeof(uint32_t);
    return size;
 
}

void MOVE_OUT_TO_MEMORY_Serialize(void* buffer, MOVE_OUT_TO_MEMORY_params* newSend) {
	uint32_t offset = 0;

	uint32_t pid = newSend -> pid;
	uint32_t tam = newSend->length;
	uint32_t newAddress = newSend->physicalAdress;
	uint32_t dataRegistro = newSend->dataRegistro;

	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &tam, &offset);
	bufferAddUint32(buffer, &newAddress, &offset);
	bufferAddUint32(buffer, &dataRegistro, &offset);
}


MOVE_OUT_TO_MEMORY_params* recv_MOVE_OUT_TO_MEMORY(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	MOVE_OUT_TO_MEMORY_params* MOVE_OUT_TO_MEMORY_Recibed = MOVE_OUT_TO_MEMORY_Deserialize(stream);

	free(stream);  // VER SI SOLUCIONA MEMORY LEAK 160 BYTES

	return MOVE_OUT_TO_MEMORY_Recibed;

}


MOVE_OUT_TO_MEMORY_params* MOVE_OUT_TO_MEMORY_Deserialize(void* stream) {

	MOVE_OUT_TO_MEMORY_params* new = malloc(sizeof(MOVE_OUT_TO_MEMORY_params));
	uint32_t offset = 0;
	
	new -> pid = bufferReadUint32(stream, &offset);
	new->length = bufferReadUint32(stream, &offset);
	new-> physicalAdress = bufferReadUint32(stream, &offset);
	new->dataRegistro = bufferReadUint32(stream, &offset);
	return new;
}
int isValidIntegerString(const char* str) {
    // Si la cadena está vacía, no es un entero válido
    if (*str == '\0') {
        return 0;
    }
    // Permitir un signo negativo opcional al inicio
    if (*str == '-') {
        str++;
    }
    // Verificar que todos los caracteres sean dígitos
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

char* intToString(int value) {
    // Determina el tamaño necesario para la cadena.
    // Contamos 1 adicional para el carácter nulo '\0'.
    int size = snprintf(NULL, 0, "%d", value) + 1;

    // Asigna memoria para la cadena.
    char* str = (char*)malloc(size);

    if (str == NULL) {
        printf("Error al asignar memoria\n");
        exit(EXIT_FAILURE);
    }

    // Convierte el entero a cadena.
    snprintf(str, size, "%d", value);

    return str;
}


COPY_STRING_params* NEW_COPY_STRING(uint32_t pid_CS, uint32_t df, uint32_t direccionFIsicaORIGEN, uint32_t tamanio) {
    // Reserva memoria para una nueva estructura io_write_or_read
    COPY_STRING_params* newIOsend_CS = malloc(sizeof(COPY_STRING_params));
    
    // Verifica si la asignación de memoria fue exitosa
    if (newIOsend_CS == NULL) {
        return NULL; // Error en la asignación de memoria
    }

    // Asigna el pid al nuevo proceso
    newIOsend_CS->pid = pid_CS;
    newIOsend_CS->dfDest = df;
	newIOsend_CS->dfOrig = direccionFIsicaORIGEN;
	newIOsend_CS->tam = tamanio;
    return newIOsend_CS;
}

void send_COPY_STRING (COPY_STRING_params* newSendIOCS, int socketClient) {

	t_package* package = packageCeate(COPY_STRING, COPY_STRING_StreamSize(newSendIOCS));
	
	t_buffer* buffer = package->buffer;

    COPY_STRING_Serialize(buffer, newSendIOCS);

	//*(buffer->offset) = 0;
	
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketClient, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);
}

uint32_t COPY_STRING_StreamSize(COPY_STRING_params* interfaceCS) {

   	uint32_t size = 0;
    size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño del PID
	size += sizeof(uint32_t); // Tamaño que se le agrega a los char* no se por que
	size += sizeof(uint32_t); // Tamaño del df
    return size;
 
}

void COPY_STRING_Serialize(void* buffer, COPY_STRING_params* newSendIOCS) {
	uint32_t offset = 0;

	uint32_t pid = newSendIOCS -> pid;
	uint32_t df = newSendIOCS->dfDest;
	uint32_t dorigen = newSendIOCS->dfOrig;
	uint32_t dtam= newSendIOCS->tam;
	bufferAddUint32(buffer, &pid, &offset);
	bufferAddUint32(buffer, &df, &offset);
	bufferAddUint32(buffer, &dorigen, &offset);
	bufferAddUint32(buffer, &dtam, &offset);
}


COPY_STRING_params* recv_COPY_STRING(int socketClient) {

	int size;
	void * stream;

	stream = recvBuffer(&size, socketClient);
	COPY_STRING_params* IO_FS_CREATE_Recibed = COPY_STRING_Deserialize(stream);

	free(stream);
	return IO_FS_CREATE_Recibed;

}

COPY_STRING_params* COPY_STRING_Deserialize(void* stream) {

	COPY_STRING_params* newIOsend = malloc(sizeof(COPY_STRING_params));
	uint32_t offset = 0;
	
	newIOsend -> pid = bufferReadUint32(stream, &offset);
	newIOsend->dfDest = bufferReadUint32(stream, &offset);
	newIOsend->dfOrig= bufferReadUint32(stream, &offset);
	newIOsend->tam = bufferReadUint32(stream, &offset);

	return newIOsend;
}

char* intToBinary8Bits(int num) {
    char* binary_str = malloc(9);
    if (binary_str == NULL) {
        log_error(logger, "No se pudo alocar memoria");
        return NULL;
    }
    num &= 0xFF;
    for (int i = 7; i >= 0; i--) {
        binary_str[7 - i] = ((num >> i) & 1) ? '1' : '0';
    }
    binary_str[8] = '\0';
    return binary_str;
}

char* intToBinary32Bits(int num) {
    char* binary_str = malloc(33);
    if (binary_str == NULL) {
        log_error(logger, "No se pudo alocar memoria");
        return NULL;
    }
    for (int i = 31; i >= 0; i--) {
        binary_str[31 - i] = ((num >> i) & 1) ? '1' : '0';
    }
    binary_str[32] = '\0';
    return binary_str;
}

int binaryToInt8Bits(char* binary_str) {
    int result = 0;
    if (strlen(binary_str) != 8) {
        //printf("La cadena no tiene 8 bits.\n");
        return -1;
    }
    for (int i = 0; i < 8; i++) {
        result = (result << 1) | (binary_str[i] - '0');
    }
    return result;
}

int binaryToInt32Bits(char* binary_str) {
    int result = 0;
    if (strlen(binary_str) != 32) {
        //printf("La cadena no tiene 32 bits.\n");
        return -1;
    }
    for (int i = 0; i < 32; i++) {
        result = (result << 1) | (binary_str[i] - '0');
    }
    return result;
}
uint32_t cadenaANumero(const char *cadena) {
    uint32_t resultado = 0;
    int longitud = strlen(cadena);

    for (int i = 0; i < longitud; i++) {
        resultado |= (uint32_t)(unsigned char)cadena[i] << (8 * (longitud - 1 - i));
    }

    return resultado;
}