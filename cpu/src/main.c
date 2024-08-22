#include "main.h"


int main(void)
{
	log_cpu = log_create("cpu.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	TLB = list_create();
	config_CPU = iniciar_config();

	msgMemory = malloc(sizeof(char) * 10);
	strcpy(msgMemory, "Memory");

	reasonForInterruption = malloc(sizeof(char)*20);

	// Primero traigo los distintos valores
	// CPU con memoria es cliente
	if ((IP_MEMORIA = config_get_string_value(config_CPU, "IP_MEMORIA")) == NULL)
		log_info(log_cpu,"Error: No se pudo obtener el valor del IP_MEMORIA \n");
	else
		log_info(log_cpu,"Valor de IP_MEMORIA: %s\n", IP_MEMORIA);

	// CP U con memoria es cliente
	if ((PUERTO_MEMORIA = config_get_string_value(config_CPU, "PUERTO_MEMORIA")) == NULL)
		log_info(log_cpu,"Error: No se pudo obtener el valor del PUERTO_MEMORIA \n");
	else
		log_info(log_cpu,"Valor de PUERTO_MEMORIA: %s\n", PUERTO_MEMORIA);

	// CPU cuando es servidor, puede estar escuchando por un dispatch
	if ((PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config_CPU, "PUERTO_ESCUCHA_DISPATCH")) == NULL)
		log_info(log_cpu,"Error: No se pudo obtener el valor del PUERTO_ESCUCHA_DISPATCH \n");
	else
		log_info(log_cpu,"Valor de PUERTO_ESCUCHA_DISPATCH: %s\n", PUERTO_ESCUCHA_DISPATCH);

	// CPU cuando es servidor, puede estar escuchando un interrupt
	if ((PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config_CPU, "PUERTO_ESCUCHA_INTERRUPT")) == NULL)
		log_info(log_cpu,"Error: No se pudo obtener el valor del PUERTO_ESCUCHA_INTERRUPT \n");
	else
		log_info(log_cpu,"Valor de PUERTO_ESCUCHA_INTERRUPT: %s\n", PUERTO_ESCUCHA_INTERRUPT);

	if ((CANTIDAD_ENTRADAS_TLB = config_get_string_value(config_CPU, "CANTIDAD_ENTRADAS_TLB")) == NULL)
		log_info(log_cpu,"Error: No se pudo obtener el valor de CANTIDAD_ENTRADAS_TLB \n");
	else
		log_info(log_cpu,"CANTIDAD_ENTRADAS_TLB: %s\n", CANTIDAD_ENTRADAS_TLB);

	if ((ALGORITMO_TLB = config_get_string_value(config_CPU, "ALGORITMO_TLB")) == NULL)
		log_info(log_cpu,"Error: No se pudo obtener el valor de ALGORITMO_TLB \n");
	else
		log_info(log_cpu,"ALGORITMO_TLB: %s\n", ALGORITMO_TLB);
	

	memoryConnection = newConnection(IP_MEMORIA, PUERTO_MEMORIA, msgMemory);
	PEDIR_TAMANIO_PAGINA();

	sem_init(&sExecutePcb, 0, 0);
	sem_init(&sCanExecuteInstruction, 0, 0);
	sem_init(&sExecuteCodeOpResources, 0, 0);
	pthread_mutex_init(&sExecuteCodeOp, NULL);
	sem_init(&sInterrupt, 0, 0);


	pthread_t tidServerDispatch;
	pthread_t tidServerInterrupt;
	pthread_t tidExecuteInstructions;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_create(&tidServerDispatch, &attr, server, PUERTO_ESCUCHA_DISPATCH);
	pthread_create(&tidServerInterrupt, &attr, serverInterrupt, NULL);
	pthread_create(&tidExecuteInstructions, &attr, executeInstrucions, NULL);
	//pthread_create(&tidClientMemory, &attr, runnerClientMemory, NULL);

	pthread_join(tidServerDispatch, NULL);
	pthread_detach(tidServerInterrupt);
	pthread_detach(tidExecuteInstructions);
	//pthread_join(tidClientMemory, NULL);

	return EXIT_SUCCESS;
}
t_config *iniciar_config(void)
{
	t_config* nuevo_config;

	nuevo_config = config_create("../cpu.config");
	if (nuevo_config == NULL)
	{
		log_info(log_cpu, "Error al cargar la config_CPU desde el archivo.\n");
		// ¡No se pudo crear el config_CPU!
		// Terminemos el programa
	}
	else
	{
		log_info(log_cpu, "Configuración cargada correctamente.\n");
	}

	return nuevo_config;
}

void iterator(char *value)
{
	log_info(log_cpu, "%s", value);
}

t_package *package(int connection)
{
	// Ahora toca lo divertido!
	char *readed;
	int lengthReaded;
	t_package *package = createPackage();
	printf("Escribi tu mensaje: ");
	readed = readline("> ");
	// Leemos y esta vez agregamos las lineas al paquete
	while ((*readed) != '\0')
	{
		lengthReaded = strlen(readed) + 1;
		addPackage(package, readed, lengthReaded);
		printf("Escribi tu mensaje: ");
		readed = readline("> ");
	}
	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	free(readed);

	return package;
}

/* void serverDispatch()
{

	int cpuServerDispatch = initServer(PUERTO_ESCUCHA_DISPATCH);
	log_info(log_cpu, "CPU Dispatch ready to receive");
	clientCpuDispatch = andHandshakeForAll(cpuServerDispatch);
	t_list *lista;
	while (1)
	{
		
		int cod_op;
		cod_op = recvOperation(clientCpuDispatch.socket);
		switch (cod_op)
		{
		case MESSAGE:
			
			recvMessage(clientCpuDispatch.socket);
			break;
		case PACKAGE:
			
			lista = recvPack(clientCpuDispatch.socket);
			log_info(log_cpu, "Server: The following values arrived:\n");
			list_iterate(lista, (void *)iterator);
			break;
		case PCB:
			printf("Llego un PCB PADRE\n");
			pcbExecute = recvPcb(clientCpuDispatch.socket);
			char *msg = malloc(sizeof(char) * 50);
			pidExecute = pcbExecute -> pid;
			strcpy(msg, "PCB Recibed. PID: ");
			strcat(msg, &pidExecute);
			log_info(log_cpu, msg);
			runnerMemory(pidExecute);
			break;
		case -1:
			log_error(log_cpu, "Server: The client disconnected");
			log_info(log_cpu, "Server: CPU Dispatch ready to receive");
			// clientCpuDispatch = waitClient(cpuServerDispatch); // Para esperar un nuevo cliente en el caso de que se desconecte el anterior
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(log_cpu, "Server: Unknown operation");
			break;
		}
	}
	printf("salgo del while\n");
}
*/
void serverInterrupt()
{
	// Espera en el semáforo
	int cpuServerInterrupt = initServer(PUERTO_ESCUCHA_INTERRUPT);
	log_info(log_cpu, "CPU Interrupt ready to receive");
	clientCpuInterrupt = andHandshakeForAll(cpuServerInterrupt);
	t_list *lista;
	while (1)
	{
		int cod_op;
		cod_op = recvOperation(clientCpuInterrupt.socket);
		switch (cod_op)
		{
		case INTERRUPTBYQUANTUM:
			//printf("ME LLEGO UNA INTERRUPCION A CPU\n");
			//printf("ME LLEGO UNA INTERRUPCION A CPU\n");
			//printf("ME LLEGO UNA INTERRUPCION A CPU\n");

			int size;
			reasonForInterruption = recvBuffer(&size, clientCpuInterrupt.socket);
			int32_t result = 0;
			sem_post(&sInterrupt);
			break;
		case -1:
			log_error(log_cpu, "Server: The client disconnected");
			log_info(log_cpu, "Server: CPU Interrupt ready to receive");
			// clientCpuInterrupt = waitClient(cpuServerInterrupt);  // Para esperar un nuevo cliente en el caso de que se desconecte el anterior
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(log_cpu, "Server: Unknown operation");
			break;
		}
	}
} 

// void serverInterrupt() {
// 	printf("INICIAMOS EL SERVER DE INTERRUPT\n");
// 	int interruptFD = initServer(PUERTO_ESCUCHA_INTERRUPT);
// 	if (interruptFD == -1) {
//         log_error(log_cpu, "Error generating socket.\n"); // Devuelve el tipo de error cuando no se puede getaddrin
//         exit(EXIT_FAILURE);
//     }
//     if (interruptFD == -2) {
//         log_error(log_cpu, "Could not assign socket to some fd INTERRUPT");
//         exit(EXIT_FAILURE);
//     }
//     if (interruptFD == -3) {
//         log_error(log_cpu, "Cannot mark socket as listening to accept connection requests");
//         exit(EXIT_FAILURE);
//     }
// 	log_info(log_cpu, "CPU Interrupt ready to receive");
// 	while(1) {
// 		printf("Vamo a espera\n");
// 		clientConnection clientFD = andHandshakeForAll(interruptFD);
// 		pthread_t tidProcessClient;
// 		pthread_attr_t attr;
// 		pthread_attr_init(&attr);
// 		pthread_create(&tidProcessClient, &attr, operate, (void*)&clientFD);
// 	}
// }

void server(char* port) {
	int serverFD = initServer(port);
	if (serverFD == -1) {
        //log_error(log_cpu, "Error generating socket. getaddrinfo: %s\n", gai_strerror(err)); // Devuelve el tipo de error cuando no se puede getaddrin
        log_error(log_cpu, "Error generating socket.\n"); // Devuelve el tipo de error cuando no se puede getaddrin
        exit(EXIT_FAILURE);
    }
    if (serverFD == -2) {
        log_error(log_cpu, "Could not assign socket to some fd DISPATCH");
        exit(EXIT_FAILURE);
    }
    if (serverFD == -3) {
        log_error(log_cpu, "Cannot mark socket as listening to accept connection requests");
        exit(EXIT_FAILURE);
    }
	log_info(log_cpu, "CPU Dispatch ready to receive");
	while(1) {
		printf("Vamo a espera\n");
		clientConnection clientFD = andHandshakeForAll(serverFD);
		pthread_t tidProcessClient;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tidProcessClient, &attr, operate, (void*)&clientFD);
	}
}

void operate(void* clientFDPtr) {
	clientConnection clientFD = *(clientConnection*)clientFDPtr;
	char* aux = detectModuleById(clientFD.client);  //VER SI SOLUCIONA MEMORY LEAK 50 BYTES
	log_info(log_cpu, "Habla %s\n", aux);  //VER SI SOLUCIONA MEMORY LEAK 50 BYTES
	free(aux);  //VER SI SOLUCIONA MEMORY LEAK 50 BYTES
	while (1) {
		kernelSocket = clientFD.socket;
		if (kernelSocket != clientFD.socket) {
			log_info(log_cpu, "No son iguales kernelSOcket y clientFD.socket\n");
		}
		
		int cod_op = recvOperation(clientFD.socket);
		//printf("[%s] Cod Op: %d\n", moduleChar(clientFD.client), cod_op);
		switch (cod_op) {
		case PCB:
			//printf("Llego un PCB PADRE\n");
			if(pcbExecute != NULL) {
				free(pcbExecute);
			}
			pcbExecute = recvPcb(clientFD.socket);
			char *msg = malloc(sizeof(char) * 50);
			pidExecute = pcbExecute -> pid;
			// strcpy(msg, "PCB Recibed. PID: ");
			// strcat(msg, &pidExecute);
			sprintf(msg, "PCB Recibed. PID: %u", pidExecute);
			//printf("pcb context %s\n", pcbExecute->context->reason);
			log_info(log_cpu, msg);
			free(msg);  // VER SI SOLUCIONA MEMORY LEAK 100 BYTES
			sem_post(&sCanExecuteInstruction);
			break;
		case INTERRUPTBYQUANTUM:
			//printf("ME LLEGO UNA INTERRUPCION A CPU\n");
			//printf("ME LLEGO UNA INTERRUPCION A CPU\n");
			//printf("ME LLEGO UNA INTERRUPCION A CPU\n");

			int size;
			char* buffer = recvBuffer(&size, clientFD.socket);
			int32_t result = 0;
			sem_post(&sInterrupt);
			break;
		case CONTINUE:
			cod_op_resource = CONTINUE;
			sem_post(&sExecuteCodeOpResources);
			break;
        case BLOCK:
                    cod_op_resource = BLOCK;
					sem_post(&sExecuteCodeOpResources);
                    break;
        case EMPTY_INSTANCE:
                    cod_op_resource = EMPTY_INSTANCE;
					sem_post(&sExecuteCodeOpResources);
                    break;
		case -1:
			log_error(log_cpu, "Server: The client disconnected");
			log_info(log_cpu, "Server: CPU Interrupt ready to receive");
			// clientCpuInterrupt = waitClient(cpuServerInterrupt);  // Para esperar un nuevo cliente en el caso de que se desconecte el anterior
			//exit(EXIT_FAILURE);
			break;
		default:
			log_warning(log_cpu, "Server: Unknown operation");
			break;
		}
	}
}

// void *runnerServerDispatch(void *param)
// {
	
// 	serverDispatch();
// 	pthread_exit(0);
// }
// void *runnerServerInterrupt(void *param)
// {

// 	serverInterrupt();
// 	pthread_exit(0);
// }
// void *runnerClientMemory(void *param)
// {
//     char *instruction;
//     int connection = newConnection(IP_MEMORIA, PUERTO_MEMORIA, msgMemory);
//     int32_t result = 0;
//     int programCounter = 0; // Inicializar el contador de programa
//     int count = 0;
//     int32_t flagCiruja;
//     while (1) {
// 		printf("este es el pcb recibdo %d\n", pidExecute);
// 		char buffer[1024];
//     	//while (recv(connection, buffer, sizeof(buffer), MSG_DONTWAIT) > 0);
// 		printf("Le aviso a memoria que estoy\n");
		
// 		conectMemoryProcess(connection, pidExecute);
// 		printf("Esperando que la memoria reciba el PID\n");
// 		recv(connection, &flagCiruja, sizeof(int32_t), MSG_WAITALL);
// 		send(connection, &result, sizeof(int32_t), 0); // Envia respuesta al cliente
// 		sendProgramCounter(connection, programCounter);
// 		log_info(log_cpu, "Program Counter enviado!");
		
// 		instruction = recvInstruction(connection);
// 		recv(connection, &flagCiruja, sizeof(int32_t), MSG_WAITALL);
//         send(connection, &result, sizeof(int32_t), 0); // Envia respuesta al cliente
// 		// Verificar si se recibió la instrucción correctamente
// 		if (instruction != NULL) {
// 			log_info(log_cpu, "recibí la instrucción %s", instruction);
// 			// Verificar que no haya interrupción previo a ejecutar
// 			executeOperations(instruction, pcbExecute);
// 			// Verificar que si llegó instruccion durante la ejecucion
// 			free(instruction); // Liberar la memoria asignada para el buffer recibido
// 			programCounter++;  // Incrementar el contador después de recibir la instrucción
// 		} else {
// 			log_error(log_cpu, "Error al recibir la instrucción");
// 			// Manejar el error aquí, posiblemente salir del bucle o terminar el programa
// 			break;
// 		}

// 		printf("-----------------%d------------\n", count);
// 		count++;
// 	}

// 	log_info(log_cpu, "Instrucciones recibidas correctamente");
// 	finishProgram(connection, NULL, NULL);
// 	pthread_exit(0);
// }

void executeInstrucions() {
	while(1) {
		char *instruction;
		//instruction = (char*)malloc(256);
		int32_t result = 0;
		// int programCounter = 0; // Inicializar el contador de programa
		int32_t flagCiruja;
		uint32_t* programCounter;
		while (1) {

			log_info(log_cpu, "Entro al semaforo: &sCanExecuteInstruction\n");
			sem_wait(&sCanExecuteInstruction);
			log_info(log_cpu, "Salgo del semaforo: &sCanExecuteInstruction\n");
			//int connection = newConnection(IP_MEMORIA, PUERTO_MEMORIA, msgMemory);
			

			programCounter = pcbExecute->cpuRegisters.PC;
			instReqToMemory newReq;
			
			//printf("Le aviso a memoria que estoy\n");
			//printf("Socket :%d\n", memoryConnection);
			//printf("PID :%d\n", pcbExecute->pid);
			//printf("PC :%d\n", *(programCounter));
			newReq.pid = pcbExecute->pid;
			newReq.programCounter = *programCounter;
			askInstruction(memoryConnection, newReq);
			// conectMemoryProcess(memoryConnection, pcbExecute->pid);
			// printf("Esperando que la memoria reciba el PID\n");
			// recv(memoryConnection, &flagCiruja, sizeof(int32_t), MSG_WAITALL);
			// send(memoryConnection, &result, sizeof(int32_t), 0); // Envia respuesta al cliente
			// sendProgramCounter(memoryConnection, (*programCounter));
			//printf("SALE DEL ASK?\n");
			//printf("REquest enviado!\n");
			instruction = recvInstruction(memoryConnection);
			
			int pc = (int) *programCounter;
			int pid = (int) pcbExecute->pid;
			logFetchInstruction(pid, pc);

			// Verificar si se recibió la instrucción correctamente
			if (instruction != NULL) {
				log_info(log_cpu, "recibí la instrucción %s\n", instruction);
				// if(strcmp(instruction, "EXIT") == 0) {
				// Liberar la memoria asignada para el buffer recibido
				// 	strcpy(pcbExecute->context->reason, "EXIT");
				// 	break;
				// } 
				(*programCounter)++;  // Incrementar el contador después de recibir la instrucción
				if(executeOperations(instruction, pcbExecute)) {
					// Si entramos aca, es por que es necesario interrumpir el PCB, por IO o Error. El mismo executeOperations guarda el motivo y demas info dentro del pcb
					log_info(log_cpu, "TAMAÑO DE BYTES DE INSTRUCCION EJECUTADA %d\n", strlen(instruction) + 1);
					log_info(log_cpu, "LA INSTRUCCIÓN EJECUTADA FUE %s\n", instruction);
					
					int offset = 0;
					char* onlyInstruct = getNextWord(instruction, &offset);
					char* params = getNextWord(instruction, &offset);
					while(instruction[offset] != '\0' && instruction[offset] != '\n'){
						strcat(params, " , ");
						char* param = getNextWord(instruction, &offset);
						strcat(params, param);
					}
					logExecutedInstruction(pid, onlyInstruct, params);
					free(onlyInstruct);  // VER SI SOLUCIONA MEMORY LEAK 100 BYTES
					free(params);  // VER SI SOLUCIONA MEMORY LEAK 100 BYTES
					free(instruction);  // VER SI SOLUCIONA MEMORY LEAK 100 BYTES
					break;
				}
				//free(instruction); // Liberar la memoria asignada para el buffer recibido
				
			} else {
				log_error(log_cpu, "Error al recibir la instrucción o finalizada la ejecucion de instrucciones");
				break;
			}
			free(instruction);
			if (sem_trywait(&sInterrupt) == 0) {
				if(strcmp(reasonForInterruption, "QUANTUM") == 0) {
					log_info(log_cpu, "Interrumpimos executeInstruction por fin de Quantum\n");
					strcpy(pcbExecute->context->reason, "QUANTUM");
					pcbExecute->quantum = 0;
				} else if(strcmp(reasonForInterruption, "CONSOLE") == 0) {
					log_info(log_cpu, "Interrumpimos executeInstruction por pedido de consola\n");
					strcpy(pcbExecute->context->reason, "CONSOLE");
				}
				//sem_post(&sInterrupt);
				break;
			}

			
			//finishProgram(connection, NULL, NULL);
			// char buffer[1024];
			// while (recv(connection, buffer, sizeof(buffer), MSG_DONTWAIT) > 0);
			
			sem_post(&sCanExecuteInstruction);
		}
		log_info(log_cpu, "SALIMOSS, devuelvo pcb pid: %u\n", pcbExecute->pid);
		log_info(log_cpu, "PC: %i\n", *(programCounter));
		log_info(log_cpu, "state: %u\n", pcbExecute->state);
		log_info(log_cpu, "quantum: %u\n", pcbExecute->quantum);
		log_info(log_cpu, "context info: %s\n", pcbExecute->context->info);
		log_info(log_cpu, "context reason: %s\n", pcbExecute->context->reason);

		sendPcb(pcbExecute, kernelSocket);
		//log_info(log_cpu, "Instrucciones recibidas correctamente");
		// free(instruction); // Liberar la memoria asignada para el buffer recibido
	}
}

int newConnection(char *ip, char *port, char *msgConnectTo)
{
	int connection;
	char *msgError = malloc(sizeof(char) * 30);
	char *msgSuccess = malloc(sizeof(char) * 30);
	strcpy(msgError, "Could not connect to ");
	strcpy(msgSuccess, "Connected to ");
	strcat(msgError, msgConnectTo);
	strcat(msgSuccess, msgConnectTo);

	connection = createConnection(ip, port, CPU);

	if (connection == -1)
	{
		log_warning(log_cpu, msgError);
		return;
	}
	log_info(log_cpu, msgSuccess);

	free(msgError);
	free(msgSuccess);
	// finishProgram(connection, NULL, NULL);
	return connection;
}

void askInstruction(int socketServer, instReqToMemory request) {
	t_package* package = packageCeate(REQUESTINSTRUCTIONS, instReqToMemorySize());
	t_buffer* buffer = package->buffer;
	instReqToMemorySerialize(buffer, request);
	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);
	send(socketServer, toSend, bytes, MSG_NOSIGNAL);
	free(toSend);
	packageDestroy(package);
}

uint32_t instReqToMemorySize() {
	uint32_t size = 0;
	size = sizeof(uint32_t) +
		   sizeof(uint32_t);
	return size;
}

int conectMemoryProcess(int socketServer, int32_t request)
{

	t_package* package = packageCeate(REQUESTINSTRUCTIONS, cpuPidAndCodStreamSize(request));
    
	t_buffer* buffer = package->buffer;
	
	cpuPidAndCodSerialize(buffer, request);

	int bytes = package->buffer->size + 2*sizeof(int);
	void* toSend = serializePackage(package, bytes);

	send(socketServer, toSend, bytes, MSG_NOSIGNAL);

	free(toSend);
	packageDestroy(package);


	// Enviar el PID del PCB al servidor
    // if (send(socketServer, &pidPCBKernel, sizeof(int32_t), 0) == -1) {
    //     log_error(log_cpu, "Error al enviar el PID del PCB al servidor");
    //     return -1; // Error al enviar el PID
    // }
    
    // int32_t result = 0;
    // // Esperar la respuesta del servidor
    // if (recv(socketServer, &result, sizeof(int32_t), MSG_WAITALL) <= 0) {
    //     log_error(log_cpu, "Error al recibir la confirmación de conexión del servidor");
    //     return -1; // Error al recibir la confirmación del servidor
    // }

    // printf("%d\n", result);

    // // Analizar la respuesta del servidor
    // if (result == 0) {
    //     log_info(log_cpu, "ConnectMemoryProces - Ok");
    //     return 0; // Conexión exitosa
    // } else {
    //     log_info(log_cpu, "ConnectMemoryProces - FAIL");
    //     return -1; // Fallo en la conexión
    // }
}

int sendProgramCounter(int socketServer, int programCounter)
{
	sendInt(programCounter, socketServer);
	int32_t result;
	recv(socketServer, &result, sizeof(int32_t), MSG_WAITALL);
	if (result == 0)
	{
		log_info(log_cpu, "sendProgramCounter - Ok");
	}
	else
	{
		log_info(log_cpu, "sendProgramCounter - FAIL");
	}
}

void PEDIR_TAMANIO_PAGINA(){
    
    int code_op = TAMANIO_PAGINA;
        
    //printf("Pido tamaño de paginas\n");
   
    //envio codigo de operacion TAMANIO_PAGINA a memoria para avisarle que me pase el tamaño de paginas
    send(memoryConnection, &code_op, sizeof(int32_t), 0);
      
    //aca espero el tamaño de paginas, pagesLength inicia en -1 (en main.h) contemplado de a entrada caso de error
    recv(memoryConnection, &pagesLength, sizeof(int32_t) , MSG_WAITALL);

    if(pagesLength != -1){
        log_info(log_cpu, "Tamaño de paginas recibido: '%d'\n", pagesLength);
        // send(memoryConnection ,1 ,sizeof(int32_t), 0); //Solo le aviso a memoria que recibí bien el tamaño de pagina
    } else {
        log_error(log_cpu, "Error al recibir tamaño de pagina");
    }

}