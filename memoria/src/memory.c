#include "memory.h"
#include <unistd.h>

int programCounter;

int main(void)
{
	log_memory = initLogger("memory.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	// Se cargan todas las variables del config
	inicializarConfig();

	initPagination();

	listProcessToMemory = list_create();
	// Inicializar el servidor
	// memoryFD = initServer(memoryPort);

	// Verificar si el servidor se inicializó correctamente
	if (memoryFD == -1)
	{
		log_error(log_memory, "Could not connect memory as server due to incorrect port");
		return EXIT_FAILURE;
	}
	log_info(log_memory, "Memory connected!! Memory ready to receive");

	// Esperar al KERNEL
	log_info(log_memory, "Waiting for EVERYONE...!\n");
	// xClientFD = andHandshakeForAll(memoryFD);
	// if(xClientFD.client == KERNEL){

	//}

	sem_init(&sCanGiveInstruction, 0, 0);
	sem_init(&sCanUploadNewInstruction, 0, 1);
	// sem_init(&sInstructionsUploaded, 0, 1);
	instructionsUploaded = 0;

	queueServer = queue_create();
	pthread_t tidServer;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tidServer, &attr, server, NULL);
	pthread_join(tidServer, NULL);

	// pthread_join(tidPagination, NULL);
}

void iterator(char *value)
{
	log_info(log_memory, "%s", value);
}

void server()
{
	int memoryFD = initServer(memoryPort);
	// printf("MEMORY FD: %d\n",memoryFD);
	if (memoryFD == -1)
	{
		// log_error(log_memory, "Error generating socket. getaddrinfo: %s\n", gai_strerror(err)); // Devuelve el tipo de error cuando no se puede getaddrin
		log_error(log_memory, "Error generating socket.\n"); // Devuelve el tipo de error cuando no se puede getaddrin
		exit(EXIT_FAILURE);
	}
	if (memoryFD == -2)
	{
		log_error(log_memory, "Could not assign socket to some fd");
		exit(EXIT_FAILURE);
	}
	if (memoryFD == -3)
	{
		log_error(log_memory, "Cannot mark socket as listening to accept connection requests");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		clientConnection clientFD = andHandshakeForAll(memoryFD);
		pthread_t tidProcessClient;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_t tidOperate;
		pthread_create(&tidProcessClient, &attr, operate, (void *)&clientFD);
		pthread_join(tidOperate, NULL);
	}
}

void operate(void *clientFDPtr)
{
	clientConnection clientFD = *(clientConnection *)clientFDPtr;
	int lastPIDused = -1;
	while (1)
	{
		int cod_op;
		// printf("Waiting Client %s...\n", moduleChar(clientFD.client));
		cod_op = recvOperation(clientFD.socket);
		// printf("[%s] Cod Op: %i\n", moduleChar(clientFD.client), cod_op);

		switch (cod_op)
		{
		case PROCESSTOMEMORY:
			sem_wait(&sCanUploadNewInstruction);
			// printf("Llego un nuevo proceso\n");

			newProcess = recvProcessToMemory(clientFD.socket); // aca la tabla de paginas

			bool findProcessToMemory(void *elem)
			{
				return findProcessToMemoryByPid(newProcess->pid, elem);
			};

			//processToMemory *newProcessToMemory = malloc(sizeof(processToMemory));  // VER SI SOLUCIONA MEMORY LEAK 24 BYTES (memoria)

			processToMemory* newProcessToMemory = list_find(listProcessToMemory, findProcessToMemory); // VER SI SOLUCIONA MEMORY LEAK 24 BYTES (memoria)
			
			if(newProcessToMemory == NULL) {
				list_add(listProcessToMemory, newProcess);
			}
	
            //printf("%s\n", newProcess->path);
            pidPCBKernel = newProcess->pid;
            //printf(" a ver el PID: %d\n", pidPCBKernel); 
            pathOfPseudoCode = newProcess->path;
            //instructions = malloc(MAX_INSTRUCTIONS * sizeof(char*));  // VER SI SOLUCIONA MEMORY LEAK 800 BYTES (memoria)
            instructions = appetiteForInstruction(pathOfPseudoCode); // Hardcodeado el path
            readArrayOfInstructions(instructions);
			//printf("\n");
			// instructionsUploaded = 1;
			//sem_post(&sInstructionsUploaded);
			log_info(log_memory, "Entra a semaforo sCanUploadNewInstruction\n");
			sem_post(&sCanUploadNewInstruction);
			log_info(log_memory, "Sale de semaforo sCanUploadNewInstruction\n");
			break;
		case REQUESTINSTRUCTIONS:
			// sendToCPU(clientFD);
			// sem_wait(&sCanGiveInstruction);
			// printf("Entra a !instructionsUploaded\n");
			//  while (!instructionsUploaded) {
			//  	// Que espere
			//  }
			// printf("Sale de !instructionsUploaded\n");
			sem_wait(&sCanUploadNewInstruction);
			int error = -1;
			int resultOK = 0;
			int resultFinish = 1;
			instReqToMemory newReq = recvInstReqToMemory(clientFD.socket);
			char *instruction = getInstruction(newReq);
			if (instruction != NULL)
			{
				int sendResult = sendInstructionToCpu(instruction, clientFD.socket);
				if (sendResult == 0)
				{
					log_info(log_memory, "PC %d - Enviada la instruccion: %s", newReq.programCounter, instructions[newReq.programCounter]);
					// 2
					// send(xClientFD.socket, &resultOK, sizeof(int), 0);
				}
				else
				{
					log_error(log_memory, "Error al enviar la instrucción al CPU");
					send(clientFD.socket, &error, sizeof(int), 0);
				}
			}
			else
			{
				for (int i = 0; instructions[i] != NULL; i++)
				{
					free(instructions[i]);
				}
				free(instructions);
			}
			usleep(getValue(answerDelay) * 1000);
			sem_post(&sCanUploadNewInstruction);

			break;
		case TAMANIO_PAGINA:
			char *lengthPageChar = malloc(strlen(lengthPage) + 1);
			strcpy(lengthPageChar, lengthPage);
			int32_t lengthPageInt = getValue(lengthPageChar);
			usleep(getValue(answerDelay) * 1000);
			send(clientFD.socket, &lengthPageInt, sizeof(int32_t), 0);
			free(lengthPageChar);  // VER SI SOLUCIONA MEMOTY LEAK 3 BYTES
			break;
		case RESIZE:

			// bool findPcb(void* elem) {
			// 	return findPcbByPid(pid, elem);
			// };
			// pthread_mutex_lock(&sListBlockeds);
			// pcb *pcbBloqued = list_find(listProcessToMemory, findPcb);
			// list_remove_element(listBlockeds, pcbBloqued);
			// pthread_mutex_unlock(&sListBlockeds);

			RESIZE_params *tamanioProceso = recv_RESIZE(clientFD.socket);

			bool findProcessToMemory2(void *elem)
			{
				return findProcessToMemoryByPid(tamanioProceso->pid, elem);
			};
			processToMemory *processInList = list_find(listProcessToMemory, findProcessToMemory2);

			int pid = tamanioProceso->pid;
			int nuevoTamanio = tamanioProceso->tamanioProceso;
			print_rectangle("NUEVO TAMAÑO");

			// uint32_t tamanioAnterior = processInList->tamanioProceso;
			// newProcess->tamanioProceso = nuevoTamanio;
			// processInList->tamanioProceso = nuevoTamanio;

			// processToMemory* processInList2 = list_find(listProcessToMemory, findProcessToMemory2);

			// printf("VALOR ANTERIOR: %d\n", tamanioAnterior);
			// printf("VALOR NUEVO: %d\n", processInList2->tamanioProceso);
			// if(nuevoTamanio > tamanioAnterior){
			// 	logProcessExpansion(pid, tamanioAnterior,nuevoTINICIAR_PROCESO scripts_memoria/MEMORIA_1amanio);
			// } else {
			// 	logProcessReduction(pid, tamanioAnterior,nuevoTamanio);
			// }
			usleep(getValue(answerDelay) * 1000);
			sendInt(resize(pid, nuevoTamanio), clientFD.socket);

			free(tamanioProceso);  // VER SI SOLUCIONA MEMORY LEAK 16 BYTES

			break;
		case IO_STDIN_READ_OP:
			IO_STDIN_READ_WRITE_MEMORY *IO_STDIN_READ = recv_IO_STDIN_READ_WRITE_MEMORY(clientFD.socket);

			uint32_t dirFisica = IO_STDIN_READ->physicalAdress;
			char *text = IO_STDIN_READ->textToMemory;

			// printf("Physical Address: %u\n", dirFisica);
			// printf("Text to Memory: %s\n", text);
			writeInPagingMemory(text, IO_STDIN_READ->pid, strlen(text), dirFisica);
			// Calcular la longitud de la cadena sin el carácter nulo
			size_t length = strlen(text);

			// Crear un void* apuntando al texto, sin el carácter nulo al final
			void *voidPointerText = malloc(length);
			if (voidPointerText == NULL)
			{
				perror("Error al asignar memoria");
				break;
			}

			// Copiar los datos al void*
			memcpy(voidPointerText, text, length);

			// datos sin '\0'
			// printf("Contenido de voidPointerText: ");
			fwrite(voidPointerText, 1, length, stdout);
			// printf("\n");

			// Mostrar el tamaño de voidPointerText
			// printf("Bytes ocupados por voidPointerText: %zu\n", length);
			usleep(getValue(answerDelay) * 1000);
			sendInt(1, clientFD.socket);
			break;
		case WRITE:
			IO_STDOUT_WRITE *newInstructionOfWrite = recv_IO_STDOUT_WRITE(clientFD.socket);
			// buscar en memoria
			char *palabraHardCodeadaLeidaDeMemoria = readInPagingMemory(newInstructionOfWrite->pid, newInstructionOfWrite->lengthToReadOrWrite, newInstructionOfWrite->physicalAdress);
			uint32_t lengthText = newInstructionOfWrite->lengthToReadOrWrite;
			printf("PALABRA HARDCODEADA: %s\n", palabraHardCodeadaLeidaDeMemoria);
			// printf("SALGO DEL READ MEMORY EN STDOUT\n");
			//  Verificacion que lengthText no sea mayor que la longitud de la palabraHardCodeadaLeidaDeMemoria

			/*if (lengthText > strlen(palabraHardCodeadaLeidaDeMemoria)) {
				lengthText = strlen(palabraHardCodeadaLeidaDeMemoria);
			}

			// nueva cadena para almacenar la subcadena
			char* subcadena = (char*)malloc(lengthText + 1); // Añadir 1 para el carácter nulo de terminación
			if (subcadena == NULL) {
				perror("Error al asignar memoria");
				break;
			}

			// Copiar los primeros 'lengthText' bytes de palabraHardCodeadaLeidaDeMemoria a subcadena
			strncpy(subcadena, palabraHardCodeadaLeidaDeMemoria, lengthText);

			// Asegurarse de que la subcadena está terminada en nulo
			subcadena[lengthText] = '\0';

			// Mostrar la subcadena resultante
			//printf("Subcadena: %s\n", subcadena);

			int subcadenaLength = strlen(subcadena) + 1;
			*/
			// printf("palabra: %s", palabraHardCodeadaLeidaDeMemoria);
			usleep(getValue(answerDelay) * 1000);
			send(clientFD.socket, &lengthText, sizeof(int), 0); // Envía el tamaño del mensaje
			if (send(clientFD.socket, palabraHardCodeadaLeidaDeMemoria, lengthText, MSG_NOSIGNAL) == -1)
			{
				log_error(log_memory, "Error sending message");
			}
			// Liberar la memoria asignada
			// free(subcadena);
			break;
		case FS_WRITE:
			IO_FS_WRITE_params *newInstructionOfFSwrite = recv_IO_FS_WRITE(clientFD.socket);
			// buscar en memoria
			char *palabraHardCodeadaLeidaDeMemoria_FS = "HEAVYMETAAAAAAALLLLL";
			uint32_t lengthText_FS = newInstructionOfFSwrite->length;

			// // Verificacion que lengthText no sea mayor que la longitud de la palabraHardCodeadaLeidaDeMemoria
			// if (lengthText_FS > strlen(palabraHardCodeadaLeidaDeMemoria_FS)) {
			// 	lengthText_FS = strlen(palabraHardCodeadaLeidaDeMemoria_FS);
			// }

			// // nueva cadena para almacenar la subcadena
			// char* subcadena_FS = (char*)malloc(lengthText_FS + 1); // Añadir 1 para el carácter nulo de terminación
			// if (subcadena_FS == NULL) {
			// 	perror("Error al asignar memoria");
			// 	break;
			// }

			// // Copiar los primeros 'lengthText' bytes de palabraHardCodeadaLeidaDeMemoria a subcadena
			// strncpy(subcadena_FS, palabraHardCodeadaLeidaDeMemoria_FS, lengthText_FS);

			// // Asegurarse de que la subcadena está terminada en nulo
			// subcadena_FS[lengthText_FS] = '\0';

			// // Mostrar la subcadena resultante
			// printf("Subcadena: %s\n", subcadena_FS);
			uint32_t pidWrite = newInstructionOfFSwrite->pid;
			uint32_t physicalAdressWrite = newInstructionOfFSwrite->physicalAdress;
			uint32_t lengthWrite = newInstructionOfFSwrite->length;
			char *subcadena_FS = readInPagingMemory(pidWrite, (int)lengthWrite, (int)physicalAdressWrite);
			int subcadenaLength_FS = strlen(subcadena_FS) + 1;
			usleep(getValue(answerDelay) * 1000);
			send(clientFD.socket, &subcadenaLength_FS, sizeof(int), 0); // Envía el tamaño del mensaje
			if (send(clientFD.socket, subcadena_FS, subcadenaLength_FS, MSG_NOSIGNAL) == -1)
			{
				log_error(log_memory, "Error sending message");
			}

			// Liberar la memoria asignada
			free(subcadena_FS);
			break;
		case FS_READ:
			IO_FS_READ_TO_MEMORY_params *newInstructionOfFSread = recv_IO_FS_READ_TO_MEMORY_(clientFD.socket);
			char *word = newInstructionOfFSread->word;
			printf("FS-READ: %s:", word);
			uint32_t pidRead = newInstructionOfFSread->pid;
			uint32_t physicalAdressRead = newInstructionOfFSread->physicalAdress;
			uint32_t lengthRead = newInstructionOfFSread->length;
			writeInPagingMemory(word, pidRead, (int)lengthRead, (int)physicalAdressRead);
			// send(clientFD.socket, 1, sizeof(int), MSG_NOSIGNAL);
			// sendInt(1, clientFD.socket);
			// print_rectangle("MEMORY FS_READ");
			// printf("WORD: %s\n", word);
			// printf("ADDRESS: %d\n", physicalAdressRead);
			usleep(getValue(answerDelay) * 1000);
			free(word);
			free(newInstructionOfFSread);
			break;
		case SOLICITUD_FRAME:
			solicitudFrameMemoria *futuroFrame = recv_SOLICITUD_FRAME_MEMORIA(clientFD.socket);
			uint32_t frame = getFrame(futuroFrame->pid, futuroFrame->paginas);

			int page = futuroFrame->paginas;
			int Pid = (int)futuroFrame->pid;
			logPageTableAccess(pid, page, (int)frame);
			usleep(getValue(answerDelay) * 1000);
			sendInt(frame, clientFD.socket);

			free(futuroFrame);
			break;
		case MOVE_IN_OP:
			MOVE_IN_TO_MEMORY_params *nuevoMoveIn = recv_MOVE_IN_TO_MEMORY(clientFD.socket);
			log_info(log_memory, "MOVE_IN IN ADDDRES: %d\n", nuevoMoveIn->physicalAdress);

			char *valorLeidoDeMemoria = readInPagingMemory(nuevoMoveIn->pid, nuevoMoveIn->length, nuevoMoveIn->physicalAdress);
			uint32_t valorLeidoDeMemoriaInt = cadenaANumero(valorLeidoDeMemoria);
			usleep(getValue(answerDelay) * 1000);
			sendInt(valorLeidoDeMemoriaInt, clientFD.socket);

			free(nuevoMoveIn);  //  VER SI SOLUCIONA MEMORY LEAK 32 BYTES
			free(valorLeidoDeMemoria);  //  VER SI SOLUCIONA MEMORY LEAK 4 BYTES

			break;
		case MOVE_OUT_OP:
			MOVE_OUT_TO_MEMORY_params *nuevoMoveOUT = recv_MOVE_OUT_TO_MEMORY(clientFD.socket);
			log_info(log_memory, "MOVE_OUT IN ADDDRES: %d\n", nuevoMoveOUT->physicalAdress);

			//char *str_OUT = intToString(nuevoMoveOUT->dataRegistro);  //  VER SI SOLUCIONA MEMORY LEAK 26 BYTES
			// Imprime la cadena y su tamaño.
			// printf("Cadena: %s\n", str_OUT);
			// printf("Tamaño: %zu bytes\n", strlen(str_OUT));

			char caracter = (char)nuevoMoveOUT->dataRegistro;
			// printf("caracter : %c\n", caracter);
			writeInPagingMemory(&caracter, nuevoMoveOUT->pid, nuevoMoveOUT->length, nuevoMoveOUT->physicalAdress);
			usleep(getValue(answerDelay) * 1000);
			free(nuevoMoveOUT);
			break;
		case COPY_STRING:
			COPY_STRING_params *nuevoCopyString = recv_COPY_STRING(clientFD.socket);
			// Nuevo tamaño deseado
			int newSize = nuevoCopyString->tam;
			char *leidoDeMemoria = readInPagingMemory(nuevoCopyString->pid, newSize, nuevoCopyString->dfOrig);
			// Verificar que el nuevo tamaño no sea mayor que la longitud de la cadena original
			if (newSize > strlen(leidoDeMemoria))
			{
				// printf("El nuevo tamaño es mayor que la longitud de la cadena original.\n");
				newSize = strlen(leidoDeMemoria);
			}

			writeInPagingMemory(leidoDeMemoria, nuevoCopyString->pid, newSize, nuevoCopyString->dfDest);
			usleep(getValue(answerDelay) * 1000);
			// printf("[COPY_STRING] Cadena: %s\n", newString);
			break;
		case -1:
			log_warning(log_memory, "Server: The client disconnected");
			log_info(log_memory, "Server: Memory ready to receive");
			// clientFD = waitClient(kernelFD); // Para esperar un nuevo cliente en el caso de que se desconecte el anterior
			// exit(EXIT_FAILURE);
			pthread_exit(0);
			break;
		default:
			log_warning(log_memory, "Server: Unknown operation");
			break;
		}
	}
}
// INICIAR_PROCESO scripts_memoria/FS_1
// INICIAR_PROCESO scripts_memoria/FS_2
// INICIAR_PROCESO scripts_memoria/FS_3
// INICIAR_PROCESO scripts_memoria/FS_4
char *getInstruction(instReqToMemory req)
{
	uint32_t pid = req.pid;
	uint32_t programCounter = req.programCounter;

	log_info(log_memory, "recibí el PID: %d\n", pid);

	if (instructions[programCounter] != NULL)
	{
		log_info(log_memory, "Instruccion: %s", instructions[programCounter]);
		// int sendResult = sendInstructionToCpu(instructions[programCounter], xClientFD.socket);
		return instructions[programCounter];
	}
	else
	{
		log_info(log_memory, "Instruccion vacia en el indice: %d", programCounter);
		// error = 0;
		// send(xClientFD.socket, &resultFinish, sizeof(int), 0);
		log_info(log_memory, "Instrucciones enviadas en su totalidad!");
		return NULL;
	}
}

void sendToCPU(clientConnection xClientFD)
{

	int32_t pidFromCPU;
	int error = -1;
	int resultOK = 0;
	int resultFinish = 1;

	while (1)
	{
		// printf("pidPCBKernel: %d\n", pidPCBKernel);
		int32_t flagCiruja;
		char buffer[1024];
		// while (recv(xClientFD.socket, buffer, sizeof(buffer), MSG_DONTWAIT) > 0);

		// 1
		pidFromCPU = recvCpuPid(xClientFD.socket);

		log_info(log_memory, "recibí el PID: %d\n", pidFromCPU);
		// 2
		send(xClientFD.socket, &flagCiruja, sizeof(int32_t), 0);
		recv(xClientFD.socket, &resultOK, sizeof(int32_t), MSG_WAITALL);
		// 2
		if (pidFromCPU == pidPCBKernel)
		{
			waitProgramCounterStrike(&programCounter, xClientFD.socket);
			log_info(log_memory, "recibí el PC: %d", programCounter);
			if (instructions[programCounter] != NULL)
			{
				log_info(log_memory, "Instruccion: %s", instructions[programCounter]);
				int sendResult = sendInstructionToCpu(instructions[programCounter], xClientFD.socket);

				if (sendResult == 0)
				{
					log_info(log_memory, "PC %d - Enviada la instruccion: %s", programCounter, instructions[programCounter]);
					break;
					// 2
					// send(xClientFD.socket, &resultOK, sizeof(int), 0);
				}
				else
				{
					log_error(log_memory, "Error al enviar la instrucción al CPU");
					send(xClientFD.socket, &error, sizeof(int), 0);
				}
			}
			else
			{
				log_info(log_memory, "Instruccion vacia en el indice: %d", programCounter);
				error = 0;
				send(xClientFD.socket, &resultFinish, sizeof(int), 0);
				log_info(log_memory, "Instrucciones enviadas en su totalidad!");
				break;
			}
		}
		else
		{
			log_error(log_memory, "Error, los procesos no coinciden");
			send(xClientFD.socket, &error, sizeof(int32_t), 0);
			break;
		}
		send(xClientFD.socket, &resultOK, sizeof(int32_t), 0);
		recv(xClientFD.socket, &flagCiruja, sizeof(int32_t), MSG_WAITALL);
		// printf("-----------------%d------------\n", programCounter);
	}

	// for (int i = 0; i < MAX_INSTRUCTIONS; ++i) {
	// 			if (instructions[i] == NULL) {
	// 				break;
	// 			}
	//     	free(instructions[i]);
	// }
	// free(instructions);
	// return EXIT_SUCCESS;
}

bool findProcessToMemoryByPid(uint32_t pid, void *ptm)
{
	processToMemory *process = (processToMemory *)ptm;
	return process->pid == pid;
}