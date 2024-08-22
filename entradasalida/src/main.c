#include <main.h>

int main(int argc, char* argv[]) 
{
	logger = initLogger("IO_log", "IO", 1, LOG_LEVEL_DEBUG);
	int result;
	int32_t flagCiruja = -1;
	int32_t action;
	sem_init(&sIO, 0, 1);
	globalOpenFiles = list_create();
	result = createInterface();

	int KerrConnection = createConnection(IP_KERNEL, PUERTO_KERNEL, IO);
	int memoryConnection = createConnection(IP_MEMORIA, PUERTO_MEMORIA, IO);
	if (KerrConnection == -1) {

		log_info(logger, "\nCould not connect TO KERNEL: '%s'\n", interfaceName);
	}
	if (memoryConnection == -1) {

		log_info(logger, "\nCould not connect TO MEMORY: '%s'\n", interfaceName);
	}


	int nameLength = strlen(interfaceName) + 1;

    send(KerrConnection , &nameLength, sizeof(int), 0);  // Envía el tamaño del mensaje
	if (send(KerrConnection, interfaceName, nameLength , MSG_NOSIGNAL) == -1) {
        log_error(logger, "Error sending message");
	
    }
	recv(KerrConnection, &flagCiruja, sizeof(int32_t), MSG_WAITALL);
	if(flagCiruja == -1) {
		log_error(logger, "No se pudo conectar la interfaz al kernel\n");
		close(KerrConnection);
	} else {
		log_info(logger, "Interfaz conectada al kernel\n");
	}
	

	int keepRunning = 1;

	while(keepRunning){
		uint32_t pid;
		int cod_op = recvOperation(KerrConnection);
		//printf("Cod Op: %i\n", cod_op);
		switch (cod_op)
		{
		case SLEEP:
			IO_GEN_SLEEP_params* params;

		   //log:	Logeo de operacion SLEEP
			params = recv_IO_GEN_SLEEP(KerrConnection);
			pid = params->pid;
			logOperation((int)pid, "SLEEP");
			if(strcmp(TIPO_INTERFAZ, "GENERIC") == 0){
				int unidad_trabajo = getValue(TIEMPO_UNIDAD_TRABAJO);
				log_info(logger, "El proceso %u entra en SLEEP durante ' %d ' segundos\n", pid, params->timeToSleep);
				usleep(params->timeToSleep * unidad_trabajo * 1000);
				log_info(logger, "El proceso %u termina de sleep\n", pid);
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
				break;
			}
			sendPid(pid, KerrConnection);
			break;
		case READ:
			IO_STDIN_READ* newInstructionOfRead;
			newInstructionOfRead = recv_IO_STDIN_READ(KerrConnection);
			pid = newInstructionOfRead->pid;
			logOperation((int)pid, "READ");			//log:	Logeo de operacion READ
			if(strcmp(TIPO_INTERFAZ, "STDIN") == 0){
				//printf("Recibí la dirección %d y el tamaño a leer %d\n", newInstructionOfRead->physicalAdress, newInstructionOfRead->lengthToReadOrWrite);
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
				break;
			}


    		// Leer la cadena desde la entrada está	ndar
			//char input[100];

			// printf("Ingrese una cadena: ");
			// if (fgets(input, sizeof(input), stdin) == NULL) {
			// 	perror("Error al leer la entrada");
			// 	return 1;
			// }
			//sem_wait(&sIO);
			char* readed;
			//char* readed = malloc(sizeof(char)*100);
			readed = readline("Ingrese una cadena: ");
			while(strlen(readed) == 0) {
				readed = readline("Ingrese una cadena valida: ");
			}

			// Eliminar el salto de línea que fgets podría haber leído
			size_t length = strlen(readed);
			if (length > 0 && readed[length - 1] == '\n') {
				readed[length - 1] = '\0';
				length--;
			}

			// Crear una nueva cadena para almacenar la subcadena
			// Añadir 1 para el carácter nulo de terminación
			char *subcadena = (char *)malloc(newInstructionOfRead->lengthToReadOrWrite + 1);
			if (subcadena == NULL) {
				log_error(logger, "Error al asignar memoria");
				return 1;
			}

			// Copiar los primeros 'tamaño' bytes de la entrada a la subcadena
			strncpy(subcadena, readed, newInstructionOfRead->lengthToReadOrWrite);

			// Asegurarse de que la subcadena está terminada en nulo
			subcadena[newInstructionOfRead->lengthToReadOrWrite] = '\0';

			// Mostrar la subcadena resultante
			//printf("Subcadena: %s\n", subcadena);

			IO_STDIN_READ_WRITE_MEMORY* newSendToMemory = NEW_IO_STDIN_READ_WRITE_MEMORY(newInstructionOfRead->physicalAdress, subcadena, pid);
			// Enviar subcadena a Memoria para que lo guarde en la dirección física.
			send_IO_STDIN_READ_WRITE_MEMORY(newSendToMemory, memoryConnection);

			int result_read;
			//printf("ENTRO A RECIV INT\n");
			recvInt(&result_read, memoryConnection);
			//printf("SALGO DEL RECIV INT\n");
			// Liberar la memoria asignada
			free(subcadena);
			//free(readed);
			//sem_post(&sIO);
			sendPid(pid, KerrConnection);
			break;
		case WRITE:

			IO_STDOUT_WRITE* newInstructionOfWrite;

				newInstructionOfWrite = recv_IO_STDOUT_WRITE(KerrConnection);
				pid = newInstructionOfWrite->pid;

			if(strcmp(TIPO_INTERFAZ, "STDOUT") == 0){

				logOperation((int)pid, "WRITE");		//log:	Logeo de operacion WRITE
				//printf("Recibí la dirección %d y el tamaño a escribir por pantalla %d\n", newInstructionOfWrite->physicalAdress, newInstructionOfWrite->lengthToReadOrWrite);
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
				break;
			}
			send_IO_STDOUT_WRITE(newInstructionOfWrite, memoryConnection);
			char* messageToRead = recvSTDOUT(memoryConnection);

			printFormattedText(messageToRead);
			free(messageToRead);
			sendPid(pid, KerrConnection);
			break;

		case FS_CREATE:
			IO_FS_CREATE_params* newInstructionOfCreate;

			newInstructionOfCreate = recv_IO_FS_CREATE(KerrConnection);
			pid = newInstructionOfCreate->pid;
			logOperation((int)pid, "FS_CREATE");		//log:	Logeo de operacion FS_CREATE
			char* fileNameToCreate = newInstructionOfCreate->fileName;
			if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){

				logCreateFile((int)pid, fileNameToCreate);

				if (fileNameToCreate != NULL && strlen(fileNameToCreate) != 0) {
					fsCreate(fileNameToCreate, pid);
    				usleep(getValue(TIEMPO_UNIDAD_TRABAJO) * 1000);
					sendPid(pid, KerrConnection);
				} else {
					log_error(logger, "Error: fileName es NULL\n");
					sendPidError(pid, KerrConnection);
				}
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
			}
			free(newInstructionOfCreate); // Liberar newInstructionOfCreate
			break;
		case FS_DELETE:
			IO_FS_DELETE_params* newInstructionOfDelete;

			newInstructionOfDelete = recv_IO_FS_DELETE(KerrConnection);
			pid = newInstructionOfDelete->pid;
			logOperation((int)pid, "FS_DELETE");		//log:	Logeo de operacion FS_DELETE
			char* fileNameToDelete = newInstructionOfDelete->fileName;
			if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){

				logDeleteFile((int)pid, fileNameToDelete);

				if (fileNameToDelete != NULL && strlen(fileNameToDelete) != 0) {
					
					fcb* fcbToClose = findFcbInGlobalOpenFiles(fileNameToDelete);
					if(fcbToClose != NULL) {
						log_info(logger, "El archivo %s se encuentra abierto, se cierra\n", fileNameToDelete);
						closeFCB(fcbToClose, fcbToClose->pid);
					}
					fsDelete(fileNameToDelete);
					usleep(getValue(TIEMPO_UNIDAD_TRABAJO) * 1000);
					sendPid(pid, KerrConnection);
				} else {
					log_error(logger, "Error: fileName es NULL\n");
					sendPidError(pid, KerrConnection);
				}			
				
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
			}
			
			free(newInstructionOfDelete); // Liberar newInstructionOfDelete
			break;
		case FS_TRUNCATE:
			IO_FS_TRUNCATE_params* newInstructionOfTruncate;

			newInstructionOfTruncate = recv_IO_FS_TRUNCATE(KerrConnection);
			pid = newInstructionOfTruncate->pid;
			logOperation((int)pid, "FS_TRUNCATE");		//log:	Logeo de operacion FS_TRUNCATE
			char* fileNameToTruncate = newInstructionOfTruncate->fileName;
			uint32_t newLength = newInstructionOfTruncate->length;
			if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){

				logTruncateFile((int)pid, fileNameToTruncate, (int)newLength);

				if (fileNameToTruncate != NULL && strlen(fileNameToTruncate) != 0) {
					if(fsTruncate(fileNameToTruncate, newLength)) {
						usleep(getValue(TIEMPO_UNIDAD_TRABAJO) * 1000);
						sendPid(pid, KerrConnection);
					} else {
						sendPidError(pid, KerrConnection);
					}
				} else {
					log_error(logger, "Error: fileName es NULL\n");
					sendPidError(pid, KerrConnection);
				}
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
				break;
			}
			free(newInstructionOfTruncate); // Liberar newInstructionOfTruncate
			break;
		case FS_WRITE:
			IO_FS_WRITE_params* newInstructionOfFS_WRITE;

			newInstructionOfFS_WRITE = recv_IO_FS_WRITE(KerrConnection);
			pid = newInstructionOfFS_WRITE->pid;
			logOperation((int)pid, "FS_WRITE");		//log:	Logeo de operacion FS_WRITE
			char* file = newInstructionOfFS_WRITE->fileName;
			uint32_t address = newInstructionOfFS_WRITE->physicalAdress;
			uint32_t newLengthWrite = newInstructionOfFS_WRITE->length;
			uint32_t pointer = newInstructionOfFS_WRITE->filePointer;
			if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){

				logWriteFile((int)pid, file, (int)newLengthWrite, uint32ToChar(pointer));

				//print_rectangle("FS_WRITE");
				send_FS_WRITE(newInstructionOfFS_WRITE, memoryConnection);
				int size;
				char* messageToWrite_FS = recvBuffer(&size, memoryConnection);;
				fsWrite(file, pointer, messageToWrite_FS);
				usleep(getValue(TIEMPO_UNIDAD_TRABAJO) * 1000);
				sendPid(pid, KerrConnection);
				free(messageToWrite_FS);
				//printf("PID: %d\n", pid);
				//printf("FILE: %s\n", file);
				//printf("ADDRESS: %d\n", address);
				//printf("LENGTH: %d\n", newLengthWrite);
				//printf("POINTER: %d\n", pointer);
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
			}
			free(newInstructionOfFS_WRITE);
			break;
		case FS_READ:
			IO_FS_READ_params* newInstructionOfFS_READ;

			newInstructionOfFS_READ = recv_IO_FS_READ(KerrConnection);
			pid = newInstructionOfFS_READ->pid;
			logOperation((int)pid, "FS_READ");		//log:	Logeo de operacion FS_READ
			char* fileNameToRead = newInstructionOfFS_READ->fileName;
			uint32_t physicalAddress = newInstructionOfFS_READ->physicalAdress;
			uint32_t dataSize = newInstructionOfFS_READ->length;
			uint32_t pointerToRead = newInstructionOfFS_READ->filePointer;
			if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){

				logReadFile((int)pid, fileNameToRead, (int)newLength, uint32ToChar(pointer));

				//print_rectangle("FS_READ");
				//printf("PID: %d\n", pid);
				//printf("FILE: %s\n", fileNameToRead);
				//printf("ADDRESS: %d\n", physicalAddress);
				//printf("LENGTH: %d\n", dataSize);
				//printf("POINTER: %d\n", pointerToRead);
				char* readed = fsRead(fileNameToRead, pointerToRead, dataSize);
				if(readed != NULL) {
					//printf("Palabra Leida %s\n", readed);
					IO_FS_READ_TO_MEMORY_params* newInstructionOfFS_READ_TO_MEMORY = NEW_IO_FS_READ_TO_MEMORY(pid, dataSize, physicalAddress, readed);
					send_FS_READ_TO_MEMORY(newInstructionOfFS_READ_TO_MEMORY, memoryConnection);
					int confirmation = 0;
					//recv(memoryConnection, &confirmation, sizeof(int), MSG_WAITALL);
					// recvInt(&confirmation, memoryConnection);
					// if(confirmation) {
					// 	printf("Se pudo escribir en memoria\n");
					// 	sendPid(pid, KerrConnection);
					// } else {
					// 	printf("No se pudo escribir en memoria\n");
					// 	sendPidError(pid, KerrConnection);
					// }
					usleep(getValue(TIEMPO_UNIDAD_TRABAJO) * 1000);
					sendPid(pid, KerrConnection);
					free(newInstructionOfFS_READ_TO_MEMORY);
				} else {
					log_error(logger, "Error: fileName es NULL\n");
					sendPidError(pid, KerrConnection);
				}
			} else {
				log_error(logger, "Esta interfaz no puede realizar esta acción\n");
				sendPidError(pid, KerrConnection);
				break;
			}
			//sendPid(pid, KerrConnection);
			free(newInstructionOfFS_READ);
		
			break;
			
		case -1:
			close(KerrConnection);
			keepRunning = 0;
		default:
			log_warning(logger, "Server: Unknown operation");
			break;
		}
	}
	
}

void sendPid(uint32_t pid, int socket) {
	t_package* package = packageCeate(FINISHED_IO, sizeof(pid));
	uint32_t offset = 0;
	bufferAddUint32(package->buffer, &pid, &offset);

	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socket, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	deletePackage(package);
}

int endsWithSlash(char *str) {
    // Verificar si la cadena no está vacía y termina con '/'
    return (str != NULL && str[strlen(str) - 1] == '/');
}

void sendPidError(uint32_t pid, int socket) {
	t_package* package = packageCeate(FINISHED_IO_ERROR, sizeof(pid));
	uint32_t offset = 0;
	bufferAddUint32(package->buffer, &pid, &offset);

	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socket, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	deletePackage(package);
}

void logOperation(int pid, char* operation){
	logString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Operacion: ");
	strcat(logString, operation);
	log_info(logger, logString);
	free(logString);
	free(pidStr);
}

void logCreateFile(int pid, char* fileName){
	logString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Crear Archivo: ");
	strcat(logString, fileName);
	log_info(logger, logString);
	free(logString);
	free(pidStr);
}

void logDeleteFile(int pid, char* fileName){
	logString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Eliminar Archivo: ");
	strcat(logString, fileName);
	log_info(logger, logString);
	free(logString);
	free(pidStr);
}

void logTruncateFile(int pid, char* fileName, int size){
	logString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	char* sizeStr = intToChar(size);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Truncar Archivo: ");
	strcat(logString, fileName);
	strcat(logString, " - Tamaño: ");
	strcat(logString, sizeStr);
	log_info(logger, logString);
	free(logString);
	free(pidStr);
	free(sizeStr);
}

void logReadFile(int pid, char* fileName, int size, char* ptr){
	logString = malloc(sizeof(char) *150);
	char* pidStr = intToChar(pid);
	char* sizeStr = intToChar(size);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Leer Archivo: ");
	strcat(logString, fileName);
	strcat(logString, " - Tamaño a Leer: ");
	strcat(logString, sizeStr);
	strcat(logString, " - Puntero Archivo: ");
	strcat(logString, ptr);
	log_info(logger, logString);
	free(logString);
	free(pidStr);
	free(sizeStr);
}

void logWriteFile(int pid, char* fileName, int size, char* ptr){
	logString = malloc(sizeof(char) *150);
	char* pidStr = intToChar(pid);
	char* sizeStr = intToChar(size);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Leer Archivo: ");
	strcat(logString, fileName);
	strcat(logString, " - Tamaño a Escribir: ");
	strcat(logString, sizeStr);
	strcat(logString, " - Puntero Archivo: ");
	strcat(logString, ptr);
	log_info(logger, logString);
	free(logString);
	free(pidStr);
	free(sizeStr);
}

void logStartCompaction(int pid){
	logString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Inicio Compactación.");
	log_info(logger, logString);
	free(logString);
	free(pidStr);
}

void logEndCompaction(int pid){
	logString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Fin Compactación.");
	log_info(logger, logString);
	free(logString);
	free(pidStr);
}



