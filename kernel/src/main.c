#include <pthread.h>
#include <time.h>
#include "main.h"
// ejecutar_script scripts_kernel/PRUEBA_IO
// INICIAR_PROCESO scripts_memoria/FS_1
// Fallout 1 Fallout 2 Fallout 3 Fallout: New Vegas Fallout 4 Fallout 76
// EJECUTAR_SCRIPT scripts_kernel/PRUEBA_DEADLOCK
// INICIAR_PROCESO scripts_memoria/MEMORIA_1
int main(void)
{
	msgMemory = malloc(sizeof(char) * 10);
	msgCpuDispatch = malloc(sizeof(char) * 15);
	msgCpuInterrupt = malloc(sizeof(char) * 15);
	strcpy(msgMemory, "Memory");
	strcpy(msgCpuDispatch, "CPU Dispatch");
	strcpy(msgCpuInterrupt, "CPU Interrupt");

	createLog();

	char *message = malloc(sizeof(char) * 100);

	t_config *config;
	char *pathConfig = malloc(sizeof(char) * 100);
	strcpy(pathConfig, "../kernel.config");
	log_info(log, pathConfig);
	config = initConfig(pathConfig);

	if (config == NULL)
	{
		strcpy(message, "No se encontro el archivo config");
		log_info(log, message);
		exit(EXIT_FAILURE);
	}

	extractAllDataConfig(config);

    // Convertir RECURSOS a un array de cadenas
  
    parseStringToStringArray(RECURSOS, &recursosArray, &recursosSize);

    // Convertir INSTANCIAS_RECURSOS a un array de enteros
    int* instanciasIntArray;
    size_t instanciasSize;

    parseStringToIntArray(INSTANCIAS_RECURSOS, &instanciasIntArray, &instanciasSize);

    // Imprimir los resultados para verificar



    // printf("Recursos:\n");
    // for (size_t i = 0; i < recursosSize; i++) {
    //     printf("%s\n", recursosArray[i]);
    // }

    // printf("Instancias de Recursos:\n");
    // for (size_t i = 0; i < instanciasSize; i++) {
    //     printf("%d\n", instanciasIntArray[i]);
    // }

	recursos = (Recurso*)malloc(recursosSize * sizeof(Recurso));

     for (size_t i = 0; i < recursosSize; i++) {
        recursos[i].recurso = recursosArray[i];
        recursos[i].instancia = instanciasIntArray[i];
    }
	



    // printf("Recursos y sus instancias:\n");
    // for (size_t i = 0; i < recursosSize; i++) {
    //     printf("Recurso: %s, Instancias: %d\n", recursos[i].recurso, recursos[i].instancia);
    // }


	pthread_mutex_init(&sQueueNews, NULL);
	pthread_mutex_init(&sQueueReadys, NULL);
	pthread_mutex_init(&sListBlockeds, NULL);
	pthread_mutex_init(&sListPcbToMemory, NULL);

	freePids = list_create();
	nextPid = 1;

	queueNews = queue_create();
	queueReadys = queue_create();
	listBlockeds = list_create();
	listResourcesPcbs = list_create();

	queueAux = queue_create();

	listPcbToMemory = list_create();

	sem_init(&sQueueReadysMax, 0, getValue(GRADO_MULTIPROGRAMACION));
	sem_init(&sQueueReadysAvaiable, 0, 0);
	sem_init(&sQueueAuxAvaiable, 0, 0);

	sem_init(&sQueueNewsAvaiable, 0, 0);

	sem_init(&sRunningProcess, 0, 1);
	sem_init(&sRunQuantum, 0, 0);

	plannerIsRunning = 0;
	sem_init(&sCanPlan, 0, plannerIsRunning);

	socketCpuDispatch = newConnection(IP_CPU, PUERTO_CPU_DISPATCH, msgCpuDispatch);
	socketCpuInterrupt = newConnection(IP_CPU, PUERTO_CPU_INTERRUPT, msgCpuInterrupt);
	socketMemory = newConnection(IP_MEMORIA, PUERTO_MEMORIA, msgMemory);

	// char* mensg = malloc(sizeof(char)*10);
	// strcpy(mensg, "Interrupt");
	// sendInterrupt(mensg, socketCpuDispatch);

	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/pseudocodigoFs.txt"
	// INICIAR_PROCESO "../tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/pseudocodigo.txt"
	// INICIAR_PROCESO /home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/pseudocodigoFs.txt
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/pseudocodigo Fs copy.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/deadlockA.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/deadlockB.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/deadlockC.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/deadlockD.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/IO_A.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/IO_B.txt"
	// INICIAR_PROCESO "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/memoria/Instruction_Store/IO_C.txt"
	// char *path = "../Instruction_Store/pseudocodigo.txt";
	// char *path2 = "../Instruction_Store/pseudocodigo 2.txt";
	// pcb *newPcb = createPcb(path);
	// pcb *newPcb2 = createPcb(path2);



	pthread_t tidServer;
	//pthread_t tidOperate;
	//pthread_t tidClientMemory;
	//pthread_t tidClientCpuDispatch;
	//pthread_t tidClientCpuInterrupt;
	pthread_t tidConsole;

	pthread_t tidProcessNewToReady;
	pthread_t tidProcessToExecute;
	pthread_t tidProcessRecivePcb;
		//pthread_t tidProcessToExit;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_create(&tidServer, &attr, runnerServer, NULL);
	//pthread_create(&tidOperate, &attr, runnerOperate, NULL);
		//pthread_create(&tidClientMemory, &attr, runnerClientMemory, NULL);
	//pthread_create(&tidClientCpuDispatch, &attr, runnerClientCpuDispatch, NULL);
	//pthread_create(&tidClientCpuInterrupt, &attr, runnerClientCpuInterrupt, NULL);

	pthread_create(&tidConsole, &attr, console, NULL);
	
	pthread_create(&tidProcessNewToReady, &attr, runnerProcessNewToReady, NULL);
	pthread_create(&tidProcessToExecute, &attr, runnerProcessToExecute, NULL);
	pthread_create(&tidProcessRecivePcb, &attr, checkResponseOfCpu, NULL);
		//pthread_create(&tidProcessToExit, &attr, runnerProcessToExit, NULL);

	pthread_join(tidServer, NULL);
	//pthread_join(tidOperate, NULL);
		//pthread_join(tidClientMemory, NULL);
	//pthread_join(tidClientCpuDispatch, NULL);
	//pthread_join(tidClientCpuInterrupt, NULL);

	//pthread_detach(tidConsole);
	pthread_detach(tidProcessNewToReady);
	pthread_detach(tidProcessToExecute);
	pthread_detach(tidProcessRecivePcb);

	free(message);
	free(pathConfig);
	queue_destroy_and_destroy_elements(queueNews, free);
	queue_destroy_and_destroy_elements(queueReadys, free);
	pthread_mutex_destroy(&sQueueReadys);
	pthread_mutex_destroy(&sQueueNews);
	pthread_mutex_destroy(&sListBlockeds);
	return EXIT_SUCCESS;
}

void createLog()
{
	char *file = malloc(sizeof(char) * 100);
	char *process_name = malloc(sizeof(char) * 100);
	strcpy(file, "./kernel_log");
	strcpy(process_name, "Kernel");
	bool is_active_console = true;
	t_log_level level = LOG_LEVEL_INFO;
	log = initLogger(file, process_name, is_active_console, level);
	free(file);
	free(process_name);
}

void server()
{
	int kernelFD = initServer(PUERTO_ESCUCHA);
	if (kernelFD == -1) {
        //log_error(logger, "Error generating socket. getaddrinfo: %s\n", gai_strerror(err)); // Devuelve el tipo de error cuando no se puede getaddrin
        log_error(log, "Error generating socket.\n"); // Devuelve el tipo de error cuando no se puede getaddrin
        exit(EXIT_FAILURE);
    }
    if (kernelFD == -2) {
        log_error(log, "Could not assign socket to some fd");
        exit(EXIT_FAILURE);
    }
    if (kernelFD == -3) {
        log_error(log, "Cannot mark socket as listening to accept connection requests");
        exit(EXIT_FAILURE);
    }
	log_info(log, "Kernel ready to receive");
	while(1) {
		printf("Vamo a espera\n");
		clientConnection clientFD = andHandshakeForAll(kernelFD);
		printf("Entro el socket %d\n", clientFD.socket);
		io* nuevaInterfaz = checkIfIos(clientFD.socket);
		printf("Salgo del checkIfIos %d\n", clientFD.socket);
		pthread_t tidProcessClient;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tidProcessClient, &attr, operate, (void*)nuevaInterfaz);
	}
}

void operate(void* nuevaInterfazPtr) {
	t_list* lista;
	while (1) {
		io* nuevaInterfaz = (io*)nuevaInterfazPtr;
		int cod_op;
		cod_op = recvOperation(nuevaInterfaz->socket);
		printf("Cod Op de %s: %i\n", nuevaInterfaz->name, cod_op);
		switch (cod_op)
		{
		case MESSAGE:
			int size;
			char* buffer = recvBuffer(&size, nuevaInterfaz->socket);
			log_info(log, "Message recived: %s", buffer);
			free(buffer);
			break;
		case PACKAGE:
			lista = recvPack(nuevaInterfaz->socket);
			log_info(log, "Server: The following values arrived:\n");
			list_iterate(lista, (void *)iterator);
			break;
		// case PCB:
		// 	pcb *newPcb = recvPcb(clientFD.socket);
		// 	char *msg = malloc(sizeof(char) * 100);
		// 	strcpy(msg, "PCB Recibed. pib: ");
		// 	uint32_t pcbExecute = newPcb->pid;
		// 	strcat(msg, &pcbExecute);
		// 	log_info(logger, msg);
		// 	break;
		case FINISHED_IO:
			printf("\nEntro semaforos sCanPla\n");
			sem_wait(&sCanPlan);
			sem_post(&sCanPlan);
			printf("\nSalgo semaforos sCanPla\n");
			printf("Algun IO terminó con algun PCB\n");
			uint32_t pidRecieved = recvCpuPid(nuevaInterfaz->socket);
			bool findPcb(void* elem) {
				return findPcbByPid(pidRecieved, elem);
			};
			desblockPcb(findPcb);
			break;
		case FINISHED_IO_ERROR:
			sem_wait(&sCanPlan);
			sem_post(&sCanPlan);
			printf("Algun IO terminó con algun PCB CON ERROR\n");
			uint32_t pidRecieved_error = recvCpuPid(nuevaInterfaz->socket);
			bool findPcb_error(void* elem) {
				return findPcbByPid(pidRecieved_error, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued_error = list_find(listBlockeds, findPcb_error);
			list_remove_element(listBlockeds, pcbBloqued_error );
			pthread_mutex_unlock(&sListBlockeds);
			printf("Se ELIMINA el proceso de pid %u\n", pcbBloqued_error -> pid);
			liberarRecursos(pidRecieved_error);
			deletePcb(pcbBloqued_error);
			break;
		case -1:
			char * message = malloc(sizeof(char)* 20);
			strcpy(message, nuevaInterfaz->name);
			strcat(message, " disconnected");
			log_warning(log, message);
			free(nuevaInterfazPtr);
			//log_info(log, "Server: Kernel ready to receive");
			// clientFD = waitClient(kernelFD); // Para esperar un nuevo cliente en el caso de que se desconecte el anterior
			//exit(EXIT_FAILURE);
			pthread_exit(0);
			break;
		default:
			log_warning(log, "Server: Unknown operation");
			break;
		}
	}
}

int newConnection(char *ip, char *port, char *msgConnectTo)
{
	int connection;
	char *msgError = malloc(sizeof(char) * 100);
	char *msgSuccess = malloc(sizeof(char) * 100);
	strcpy(msgError, "Could not connect to ");
	strcpy(msgSuccess, "Connected to ");
	strcat(msgError, msgConnectTo);
	strcat(msgSuccess, msgConnectTo);

	connection = createConnection(ip, port, KERNEL);

	if (connection == -1)
	{
		log_warning(log, msgError);
		return;
	}
	log_info(log, msgSuccess);

	free(msgError);
	free(msgSuccess);
	// finishProgram(connection, NULL, NULL);
	return connection;
}

void *runnerServer(void *param)
{
	server();
	pthread_exit(0);
}

// void* runnerOperate(void* param) {
// 	operate();
// 	pthread_exit(0);
// }

// void* runnerClientMemory(void* param) {
	//int connection = newConnection(IP_MEMORIA, PUERTO_MEMORIA, msgMemory);
	//finishProgram(connection, NULL, NULL);
// 	pthread_exit(0);
// }

// void *runnerClientCpuDispatch(void *param)
// {
// 	printf("Dispatcheando\n");
// 	char *path = "../Instruction_Store/pseudocodigo.txt";
// 	pcb *newPcb = createPcb(2, path);
// 	printf("PID reado %d\n", newPcb->pid);
// 	bool findPcbtoMemory(void* elem) {
// 		return findPcbToMemoryByPid(newPcb->pid, elem);
// 	};
// 	processToMemory *newProcessToMemory = list_find(listPcbToMemory, findPcbtoMemory);
// 	sendProcessToMemory(newProcessToMemory, socketMemory);

// 	sendPcb(newPcb, socketCpuDispatch);
// 	pthread_exit(0);
// }

// void *runnerClientCpuInterrupt(void *param)
// {
	//int connection = newConnection(IP_CPU, PUERTO_CPU_INTERRUPT, msgCpuInterrupt);
	//finishProgram(connection, NULL, NULL);
// 	pthread_exit(0);
// }

void* runnerProcessNewToReady(void* param) {
	while(1) {
		// wait() para las distintas listas
		sem_wait(&sQueueNewsAvaiable);
		sem_wait(&sQueueReadysMax);
		sem_wait(&sCanPlan);
		sem_post(&sCanPlan);
		pthread_mutex_lock(&sQueueNews);
		pthread_mutex_lock(&sQueueReadys);
		
		pcb *newPcbReady = queue_peek(queueNews);
		queue_pop(queueNews);
		pthread_mutex_unlock(&sQueueNews);

		newPcbReady->state = READY;

		int pid = newPcbReady->pid;
		char* state = stateToChar(newPcbReady);
		logChangeStateProcess(pid, "NEW", state);     //LOG:  Cambio de estado de NEW -> READY
		queue_push(queueReadys, newPcbReady);

		logReadyEntryProcess();		//LOG:  Movimiento de proceso a cola de READY

		pthread_mutex_unlock(&sQueueReadys);
		sem_post(&sQueueReadysAvaiable);
		// fgets(input, sizeof(input), stdin);  // Leer la entrada del usuario

		// printf("Continuando...\n");
	}	
	pthread_exit(0);
}

// void* runnerProcessToExit(void* param) {

// 	pthread_exit(0);
// }

void* runnerProcessToExecute(void* param) {
	char input[100];  // Crear un buffer para almacenar la entrada del usuario

		// printf("Presiona ENTER para continuar...\n");
		// fgets(input, sizeof(input), stdin);  // Leer la entrada del usuario

		// printf("Continuando...\n");

		// Aquí puedes poner el resto de tu código
	if(strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0) {
		chosenPlanner = FIFO;
		printf("Se reconoce como algoritmo de planificacion %s\n", ALGORITMO_PLANIFICACION);
	} else if (strcmp(ALGORITMO_PLANIFICACION, "RR") == 0) {
		chosenPlanner = RR;
		printf("Se reconoce como algoritmo de planificacion %s\n", ALGORITMO_PLANIFICACION);
	} else if (strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0){
		chosenPlanner = VRR;
		printf("Se reconoce como algoritmo de planificacion %s\n", ALGORITMO_PLANIFICACION);
	}else {
		chosenPlanner = FIFO;
		printf("Se reconoce como algoritmo de planificacion por defecto\n");
	}
	while(1) {
		sem_wait(&sCanPlan);
		sem_post(&sCanPlan);
		int interrupted;
		uint32_t maxQuantum;
		struct timespec req, rem;

		switch (chosenPlanner)
		{
		case FIFO:
			sem_wait(&sRunningProcess);
			sem_wait(&sQueueReadysAvaiable);
			fifo();
			break;
		case RR:
			sem_wait(&sRunningProcess);
			sem_wait(&sQueueReadysAvaiable);
			fifo();
			interrupted = 1;
			quantum = 0;
			maxQuantum = (uint32_t)atoi(QUANTUM);

            req.tv_sec = 0;
            req.tv_nsec = 1000000L; // 1 milisegundo

			while(quantum < maxQuantum) {
				// if (sem_trywait(&sRunningProcess) == 0) {
				// 	interrupted = 0;
                //     sem_post(&sRunningProcess); // Liberar el semáforo
                //     break;
                // }
				// quantum++;
				if (sem_trywait(&sRunningProcess) == 0) {
                    // No hay proceso ejecutándose
					interrupted = 0;
                    sem_post(&sRunningProcess); // Liberar el semáforo
                    break;
                }
                // Esperar 1 milisegundo
                nanosleep(&req, &rem);
                quantum++;
			}
			//Interrupt
			if(interrupted)
				interruptProcessCpu("QUANTUM");
			break;
		case VRR:
			pcbToExecute;
			sem_wait(&sRunningProcess);
			sem_wait(&sQueueReadysAvaiable);
			if(queue_size(queueAux) > 0) {
				pthread_mutex_lock(&sQueueAux);
				pcbToExecute = queue_pop(queueAux);
				printf("Se va a ejecutar el pid %i con prioridad sobre la cola de listos \n", pcbToExecute->pid);
				pthread_mutex_unlock(&sQueueAux);
				printf("Actualmente el Quantum es %i \n", pcbToExecute->quantum);
				executePcb(pcbToExecute);
			} else {
				pcbToExecute = fifo();
			}

			interrupted = 1;
			quantum = pcbToExecute->quantum;
			maxQuantum = (uint32_t)atoi(QUANTUM);

            req.tv_sec = 0;
            req.tv_nsec = 1000000L; // 1 milisegundo

			while(quantum < maxQuantum) {
				//printf("Quantum al entrar al ciclo while %i \n", pcbToExecute->quantum);
				if (sem_trywait(&sRunningProcess) == 0) {
                    // No hay proceso ejecutándose
					interrupted = 0;
					pcbToExecute->quantum = quantum;
                    sem_post(&sRunningProcess); // Liberar el semáforo
					printf("Actualmente se ejecutaron %i rafagas de quantum \n", pcbToExecute->quantum);
                    break;
                }
                // Esperar 1 milisegundo
                nanosleep(&req, &rem);
                quantum++;
				pcbToExecute->quantum = quantum;
			}
			printf("Quantum al salir del while %i \n", pcbToExecute->quantum);
			//Interrupt
			if(interrupted)
				interruptProcessCpu("QUANTUM");
			break;
		default:
			fifo();
			break;
		}
	}
	pthread_exit(0);
}

pcb* fifo() {
	pthread_mutex_lock(&sQueueReadys);
	pcb* pcbToExecute = queue_pop(queueReadys);
	printf("Se va a ejecutar el pid %i\n", pcbToExecute->pid);
	pthread_mutex_unlock(&sQueueReadys);

	executePcb(pcbToExecute);
	return pcbToExecute;
	//checkResponseOfCpu();
}

void executePcb(pcb* pcbToExecute) {
	bool findPcbtoMemory(void* elem) {
		return findPcbToMemoryByPid(pcbToExecute->pid, elem);
	};
	processToMemory *newProcessToMemory = list_find(listPcbToMemory, findPcbtoMemory);
	printf("Enviamos PCB pid: %i\n", pcbToExecute->pid);

	pcbToExecute->state = EXEC;

	int pid = (int)pcbToExecute->pid;
	char* state = stateToChar(pcbToExecute);
	logChangeStateProcess(pid, "READY", state);     //LOG:  Cambio de estado de READY -> EXEC

	currentyRunningPcb = pcbToExecute;
	printf("Pid Ejecutandose %u\n", currentyRunningPcb->pid);

	sendPcb(pcbToExecute, socketCpuDispatch);
	sendProcessToMemory(newProcessToMemory, socketMemory);
	Recursos_de_PID* recursosDePID = buscarRecursosDePID(listResourcesPcbs, pid);
	printRecursosDePID(recursosDePID);

}

void interruptProcessCpu(char* reason) {
	char* mensg = malloc(sizeof(char)*10);
	strcpy(mensg, reason);
	//sendInterrupt(mensg, socketCpuInterrupt);
	sendInterrupt(mensg, socketCpuInterrupt);
	// recvOperation(socketCpuInterrupt);
	// pcb* pcbInterrupted = recvPcb(socketCpuInterrupt);
	// printf("Pid PCB interrumpido: %u\n", pcbInterrupted->pid);
	// pthread_mutex_lock(&sQueueReadys);
	// queue_push(queueReadys, pcbInterrupted);
	// pthread_mutex_unlock(&sQueueReadys);
	// sem_post(&sQueueReadysAvaiable);
	// sem_post(&sRunningProcess);
	free(mensg);
}

void blockPcb(pcb* pcbToBlock) {
	//pcbToBlock->quantum = pcbToExecute->quantum;
	pthread_mutex_lock(&sListBlockeds);
	
	char* state = stateToChar(pcbToBlock);
	logChangeStateProcess(pcbToBlock->pid, state, "BLOCKED");  //LOG:  Cambio de estado EXEC -> BLOCKED
	pcbToBlock->state = BLOCKED;

	list_add(listBlockeds, pcbToBlock);
	pthread_mutex_unlock(&sListBlockeds);
}

pcb* desblockPcb(bool (*pcbSearchCriteria)(void*)) {
	pthread_mutex_lock(&sListBlockeds);
	pcb *pcbBloqued = list_find(listBlockeds, pcbSearchCriteria);
	list_remove_element(listBlockeds, pcbBloqued);
	pthread_mutex_unlock(&sListBlockeds);
	pcbBloqued->state = READY;

	char* state = stateToChar(pcbBloqued);
	logChangeStateProcess(pcbBloqued->pid, "BLOCKED", state);			//LOG:  Cambio de estado de BLOCKED -> READY

	if ( ( strcmp(ALGORITMO_PLANIFICACION,"VRR") == 0 ) && ( pcbBloqued->quantum < (uint32_t)atoi(QUANTUM) ) ) {
		pthread_mutex_lock(&sQueueAux);
		queue_push(queueAux, pcbBloqued);

		logReadyPrioEntryProcess();		//LOG:  Movimiento de proceso a cola de READY con prioridad

		pthread_mutex_unlock(&sQueueAux);
		sem_post(&sQueueReadysAvaiable);
		
		printf("Se manda PCB pid %u a Ready con prioridad\n", pcbBloqued->pid);
		printf("Quantum actual %u\n", pcbBloqued->quantum);

	} else {
		pthread_mutex_lock(&sQueueReadys);
		queue_push(queueReadys, pcbBloqued);

		logReadyEntryProcess();		//LOG:  Movimiento de proceso a cola de READY

		pthread_mutex_unlock(&sQueueReadys);
		sem_post(&sQueueReadysAvaiable);
		printf("Se manda PCB pid %u a Ready\n", pcbBloqued->pid);

	}
	return pcbBloqued;
}


void extractAllDataConfig(t_config *config)
{
	PUERTO_ESCUCHA = getConfigValue(config, "PUERTO_ESCUCHA");
	IP_MEMORIA = getConfigValue(config, "IP_MEMORIA");
	PUERTO_MEMORIA = getConfigValue(config, "PUERTO_MEMORIA");
	IP_CPU = getConfigValue(config, "IP_CPU");
	PUERTO_CPU_DISPATCH = getConfigValue(config, "PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = getConfigValue(config, "PUERTO_CPU_INTERRUPT");
	ALGORITMO_PLANIFICACION = getConfigValue(config, "ALGORITMO_PLANIFICACION");
	QUANTUM = getConfigValue(config, "QUANTUM");
	RECURSOS = getConfigValue(config, "RECURSOS");
	INSTANCIAS_RECURSOS = getConfigValue(config, "INSTANCIAS_RECURSOS");
	GRADO_MULTIPROGRAMACION = getConfigValue(config, "GRADO_MULTIPROGRAMACION");
	PATH_PRUEBAS = getConfigValue(config, "PATH_PRUEBAS");
	PATH_PRUEBAS = !endsWithSlash(PATH_PRUEBAS) ? strcat(PATH_PRUEBAS, "/") : PATH_PRUEBAS;
}

void console() {
	char* readed;
	t_queue* wordList; // Puntero que guarda punteros, y cada puntero que guarda es cada palabra por separado escrito en la consola
	const char delimiter[] = " ";
	printf("Bienvenidos a la consola interactva\n");
	do {
		readed = readline("> ");
		if (strlen(readed) != 0) {
			wordList = splitString(readed, delimiter);
			if(wordList != NULL) {
				interpetAndExecCommand(wordList, PATH_PRUEBAS);
			} else {
				printf("Revise la instrucción\n");
			}	
		}
		free(readed);
			
	//} while((*readed) != '\0');
	} while(1);
	queue_destroy(wordList);
}

bool findPcbToMemoryByPid(uint32_t pid, void* ptm) {
	processToMemory* process = (processToMemory*)ptm;
	return process->pid == pid;
}

bool findPcbByPid(uint32_t pid, void* voidPcb) {
	pcb* pcbInList = (pcb*)voidPcb;
	return pcbInList->pid == pid;
}


bool findPcbByContext(char* info, void* voidPcb) {
    pcb* pcbInList = (pcb*)voidPcb;
    return strcmp(pcbInList->context->info, info) == 0;
}

pcb* createPcb(char* path) {
	pcb* pcb = newPcb(getNewPid());
	processToMemory* pcbMemory = newProcessToMemory(pcb->pid, path);
	if (pcb == NULL) {
		log_error(log, "No se pudo crear un PCB nuevo");
	}
	pthread_mutex_lock(&sListPcbToMemory);
	list_add(listPcbToMemory, pcbMemory);
	pthread_mutex_unlock(&sListPcbToMemory);
	pthread_mutex_lock(&sQueueNews);
	queue_push(queueNews, pcb);

	int newPid = (int)pcb->pid;
	logNewProcess(newPid);

	pthread_mutex_unlock(&sQueueNews);
	sem_post(&sQueueNewsAvaiable);

	Recursos_de_PID* recursosPID = crearRecursosDePID(newPid);
	list_add(listResourcesPcbs, recursosPID);
	return pcb;
}

void deletePcb(pcb* pcbToDelete) {
	bool findPcbtoMemory(void* elem) {
		return findPcbToMemoryByPid(pcbToDelete->pid, elem);
	};
	pthread_mutex_lock(&sListPcbToMemory);
	list_remove_by_condition(listPcbToMemory, findPcbtoMemory);
	pthread_mutex_unlock(&sListPcbToMemory);
	releasePid(pcbToDelete->pid);
	destroyPcb(pcbToDelete);
	printf("PCB DELETED\n");
	sem_post(&sQueueReadysMax);
}


// Función para obtener un nuevo Pid
int getNewPid() {
    if (list_size(freePids)) {
        // Si hay Pids libres, toma el primero de la lista
        int id = list_remove(freePids, 0);
        return id;
    } else {
        // Si no hay Pids libres, genera un nuevo Pid
        return nextPid++;
    }
}

void releasePid(int pid) {
	list_add(freePids, pid);
}

char* stateToChar(pcb *pcb) {

	int state = pcb->state;
    switch (state) {
        case NEW: 
			return "NEW";
        case READY: 
			return "READY";
        case BLOCKED: 
			return "BLOCKED";
        case EXEC: 
			return "EXEC";
        case EXIT: 
			return "EXIT";
		default:
			return "UNKNOWN";
    }
}

void logNewProcess(int pid) {
	logString = malloc(sizeof(char) *200);
	char* pidStr = intToChar(pid);
	strcpy(logString, "Se crea el proceso ");
	strcat(logString, pidStr);
	strcat(logString, " en NEW");
	log_info(log, logString);
	free(logString);
	free(pidStr);
}
void logExitProcess(int pid, char* reason) {
	logString = malloc(sizeof(char) *200);
	char* pidStr = intToChar(pid);
	strcpy(logString, "Finaliza el proceso ");
	strcat(logString, pidStr);
	strcat(logString, " - Motivo: ");
	strcat(logString, reason);
	log_info(log, logString);
	free(logString);
	free(pidStr);
}
void logChangeStateProcess(int pid, char* previousState, char* newState) {
	logString = malloc(sizeof(char) *200);
	// char* pidStr = intToChar(pid);
	// strcpy(logString, "PID: ");
	// strcat(logString, pidStr);
	// strcat(logString, " - Estado Anterior: ");
	// strcat(logString, previousState);
	// strcat(logString, " - Estado Actual: ");
	// strcat(logString, newState);
	sprintf(logString, "PID: %i - Estado Anterior: %s - Estado Actual: %s", pid, previousState, newState);
	log_info(log, logString);
	free(logString);
	// free(pidStr);
}
void logBlockingReasonProcess(int pid, char* bloquedFor) {
	logString = malloc(sizeof(char) *200);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Bloqueado por: ");
	strcat(logString, bloquedFor);
	log_info(log, logString);
	free(logString);
	free(pidStr);
}
void logFinishedQuantumProcess(int pid) {
	logString = malloc(sizeof(char) *200);
	char* pidStr = intToChar(pid);
	strcpy(logString, "PID: ");
	strcat(logString, pidStr);
	strcat(logString, " - Desalojado por fin de Quantum");
	log_info(log, logString);
	free(logString);
	free(pidStr);
}
void logReadyPrioEntryProcess() {
	logString = malloc(sizeof(char) *200);
	strcpy(logString, "Cola Ready con Prioridad: [");
	queue_iterate(queueAux, putPids);
	size_t len = strlen(logString);
    if (len > 2 && logString[len - 1] == ',') {
        logString[len - 1] = ']';
    }
	log_info(log, logString);
	free(logString);
}
void logReadyEntryProcess() {
	logString = malloc(sizeof(char) *200);
	strcpy(logString, "Cola Ready: [");
	queue_iterate(queueReadys, putPids);
	size_t len = strlen(logString);
    if (len > 2 && logString[len - 1] == ',') {
        logString[len - 1] = ']';
    }
	log_info(log, logString);
	free(logString);
}

void putPids(void* info) {
	pcb* pcbInfo = (pcb*) info;
	char* pidStr = malloc(sizeof(char) *20);
	sprintf(pidStr, " %u ,", pcbInfo->pid);
	strcat(logString, pidStr);
	free(pidStr);
}

void recivePcb() {
	printf("Espero PCB... \n");
	recvOperation(socketCpuDispatch);
	pcb *pcbReceived = recvPcb(socketCpuDispatch);
	printf("Me devolvio PCB PID: %u\n", pcbReceived->pid);
	printf("con PC: %u\n", pcbReceived->cpuRegisters.PC);
	printf("reason: %s\n", pcbReceived->context->reason);
	if(strcmp(pcbReceived->context->reason, "EXIT") == 0) {
		printf("PCB Se fue a EXIT\n");
	} else if(strcmp(pcbReceived->context->reason, "QUANTUM") == 0) {
		printf("Llego al QUANTUM\n");
	} else if(strcmp(pcbReceived->context->reason, "WAIT") == 0) {
		printf("VINO INSTRUCCION WAIT\n");
	} else {
		printf("No se por que me devolvio PCB\n");
	}
}

void checkResponseOfCpu() {
    int cod_op;
    int index;
    char* resource;
    while (1) {
        printf("Waiting recb in checkResponseOfCpu\n");
        cod_op = recvOperation(socketCpuDispatch);
        printf("Cod Op: %i\n", cod_op);
        switch (cod_op) {
        case PCB: {
			currentyRunningPcb = NULL;
            pcb *pcbReceived = recvPcb(socketCpuDispatch);
            printf("Me devolvio PCB PID: %u\n", pcbReceived->pid);
            printf("con PC: %u\n", (*pcbReceived->cpuRegisters.PC));
            printf("reason: %s\n", pcbReceived->context->reason);
            char* reason = pcbReceived->context->reason;
            char* info = pcbReceived->context->info;

            if (strcmp(reason, "EXIT") == 0) {
                printf("PCB Se fue a EXIT\n");
                deletePcb(pcbReceived);
            } else if (strcmp(reason, "QUANTUM") == 0) {
                pthread_mutex_lock(&sQueueReadys);
                pcbReceived->state = READY;
                queue_push(queueReadys, pcbReceived);

				logReadyEntryProcess();		//LOG:  Movimiento de proceso a cola de READY

                pthread_mutex_unlock(&sQueueReadys);
                sem_post(&sQueueReadysAvaiable);
            } else if (strcmp(reason, "IO") == 0) {
                printf("El PCB salio por IO\n");
				blockPcb(pcbReceived);

                ioParams paramsIo;
                paramsIo.operation = info;
                paramsIo.pid = pcbReceived->pid;

                pthread_t tidIo;
                pthread_attr_t attr;

                pthread_attr_init(&attr);
                pthread_create(&tidIo, &attr, ioFunction, (void*)&paramsIo);
                pthread_detach(tidIo);
            } else if (strcmp(reason, "WAIT") == 0) {
                blockPcb(pcbReceived);
            } else if (strcmp(reason, "ERROR") == 0) {
                printf("El PCB salio por un ERROR\n");
                printf("%s\n", pcbReceived->context->info);
                deletePcb(pcbReceived);
            } else if(strcmp(reason, "CONSOLE") == 0) {
				printf("PCB interrumpido por Consola\n");
                deletePcb(pcbReceived);
			} else {
                printf("No se por que me devolvio PCB\n");
            }
            sem_post(&sRunningProcess);
            break;
        }
        case WAIT: {

			uint32_t pid_wait;
            resource = recvResourceWait(socketCpuDispatch);
			recvInt(&pid_wait, socketCpuDispatch);
            index = findIndexOfString(recursosArray, recursosSize, resource);
            if (index != -1) {
                recursos[index].instancia--;
                printf("El recurso %s tiene %d instancias.\n", resource, recursos[index].instancia);

				 // Buscar en la lista resourcesPCB
				bool encontrarPorPID(void* elem) {
					Recursos_de_PID* recursos = (Recursos_de_PID*)elem;
					return recursos->pid == pid_wait;
				}

				Recursos_de_PID* recursosDePID = list_find(listResourcesPcbs, encontrarPorPID);

				if (recursosDePID != NULL) {
					// Duplicar la cadena de recurso
					char* recursoCopia = strdup(resource);
					if (recursoCopia == NULL) {
						perror("Error duplicating string");
						exit(EXIT_FAILURE);
					}

					// Añadir la copia del recurso a la lista de recursos del PID
					list_add(recursosDePID->recursos, recursoCopia);
				}
            } else {
                printf("El recurso %s no se encuentra en el array.\n", resource);
                printf("El PROCESO SE VA A EXIT\n");
                sendInt(EMPTY_INSTANCE, socketCpuDispatch);
                free(resource);
                break;
            }
            if (recursos[index].instancia < 0) {
                printf("No quedaron instancias se loquea el proceso\n");
                sendInt(BLOCK, socketCpuDispatch);
            } else {
                sendInt(CONTINUE, socketCpuDispatch);
            }
            free(resource);
            break;
        }
        case SIGNAL: 
			uint32_t pid_SIGNAL;
            resource = recvResourceSignal(socketCpuDispatch);
			recvInt(&pid_SIGNAL, socketCpuDispatch);
            index = findIndexOfString(recursosArray, recursosSize, resource);
            if (index != -1) {
                recursos[index].instancia++;
                printf("El recurso %s tiene %d instancias.\n", resource, recursos[index].instancia);
						
				Recursos_de_PID* recursosDePID = buscarRecursosDePID(listResourcesPcbs, pid_SIGNAL);
				if (recursosDePID != NULL) {
					removerRecurso(recursosDePID, resource);
				}


            } else {
                printf("El recurso %s no se encuentra en el array.\n", resource);
                printf("El PROCESO SE VA A EXIT\n");
                sendInt(EMPTY_INSTANCE, socketCpuDispatch);
                break;
            }
            if (recursos[index].instancia >= 0) {
                bool findPcb(void* elem) {
                    return findPcbByContext(resource , elem);
                }
                pthread_mutex_lock(&sListBlockeds);
                pcb *pcbBloqued = list_find(listBlockeds, findPcb);
                if (pcbBloqued == NULL) {
                    printf("PCB not found in blocked list\n");
                    pthread_mutex_unlock(&sListBlockeds);
					sendInt(CONTINUE, socketCpuDispatch);
                    break;
                } else {
					pthread_mutex_unlock(&sListBlockeds);
                    desblockPcb(findPcb);
					sendInt(CONTINUE, socketCpuDispatch);
                }
            }
            free(resource);
            break;
        
        case -1:
            exit(EXIT_FAILURE);
            break;
        default:
            log_warning(log, "Server: Unknown operation");
            break;
        }
        printf("Sale del break?\n");
    }
}


void ioFunction(void* params) {
	ioParams* paramsIo = (ioParams*)params;
	char* bufferIO;
	int offset = 0;
	uint32_t pid = paramsIo->pid;
	char* operation = paramsIo->operation;
	bufferIO = getNextWord(operation, &offset);
	sem_wait(&sCanPlan);
	sem_post(&sCanPlan);
	if(strcmp(bufferIO, "IO_GEN_SLEEP") == 0){

		char* interface = getNextWord(operation, &offset);
		char* timeToSleepChar = getNextWord(operation, &offset);
		int32_t timeToSleep = getValue(timeToSleepChar);
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_GEN_SLEEP_params* ioSend = NEW_IO_GEN_SLEEP(pid, timeToSleep);
			send_IO_GEN_SLEEP(ioSend, interfaceAndSocket->socket);
		} else {
			free(bufferIO);
			free(interface );
			free(timeToSleepChar);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
			//break;
		}
		free(bufferIO);
		free(interface);
		free(timeToSleepChar);
	}

	if(strcmp(bufferIO, "IO_STDIN_READ") == 0) {

		char* interface = getNextWord(operation, &offset);
		char* physicalAddresChar = getNextWord(operation, &offset);
		uint32_t physicalAddress = getValue(physicalAddresChar);
		char* lengthChar = getNextWord(operation, &offset);
		uint32_t length = getValue(lengthChar);
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_STDIN_READ* ioSend = NEW_IO_STDIN_READ(pid, physicalAddress, length);
			send_IO_STDIN_READ(ioSend, interfaceAndSocket->socket);
			
		} else {
			free(bufferIO);
			free(interface);
			free(physicalAddresChar);
			free(lengthChar);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
		}
		free(bufferIO);
		free(interface);
		free(physicalAddresChar);
		free(lengthChar);
		
	}
	if(strcmp(bufferIO, "IO_STDOUT_WRITE") == 0){

		char* interface = getNextWord(operation, &offset);
		char* physicalAddresChar = getNextWord(operation, &offset);
		uint32_t physicalAddress = getValue(physicalAddresChar);
		char* lengthChar = getNextWord(operation, &offset);
		uint32_t length = getValue(lengthChar);
		
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_STDOUT_WRITE* ioSend = NEW_IO_STDOUT_WRITE(pid, physicalAddress, length);
			send_IO_STDOUT_WRITE(ioSend, interfaceAndSocket->socket);
			
		} else {
			free(bufferIO);
			free(interface);
			free(physicalAddresChar);
			free(lengthChar);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
		}
		free(bufferIO);
		free(interface);
		free(physicalAddresChar);
		free(lengthChar);
	} if(strcmp(bufferIO, "IO_FS_CREATE") == 0){

		char* interface = getNextWord(operation, &offset);
		char* fileName = getNextWord(operation, &offset);
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_FS_CREATE_params* ioSend = NEW_IO_FS_CREATE(pid, fileName);
			send_FS_CREATE(ioSend, interfaceAndSocket->socket);
		} else {
			free(bufferIO);
			free(interface );
			free(fileName);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
			//break;
		}
		free(bufferIO);
		free(interface);
		free(fileName);
	} if(strcmp(bufferIO, "IO_FS_DELETE") == 0){

		char* interface = getNextWord(operation, &offset);
		char* fileName = getNextWord(operation, &offset);
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_FS_DELETE_params* ioSend = NEW_IO_FS_DELETE(pid, fileName);
			printf("IOSEND-FILENAME: %s\n", fileName);
			send_FS_DELETE(ioSend, interfaceAndSocket->socket);
		} else {
			free(bufferIO);
			free(interface );
			free(fileName);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
			//break;
		}
		free(bufferIO);
		free(interface);
		free(fileName);
	} if(strcmp(bufferIO, "IO_FS_TRUNCATE") == 0){

		char* interface = getNextWord(operation, &offset);
		char* fileName = getNextWord(operation, &offset);
		uint32_t newLength = getValue(getNextWord(operation, &offset));
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_FS_TRUNCATE_params* ioSend = NEW_IO_FS_TRUNCATE(pid, fileName, newLength);
			printf("IOSEND-FILENAME: %s\n", fileName);
			send_FS_TRUNCATE(ioSend, interfaceAndSocket->socket);
		} else {
			free(bufferIO);
			free(interface );
			free(fileName);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
			//break;
		}
		free(bufferIO);
		free(interface);
		free(fileName);
	} if(strcmp(bufferIO, "IO_FS_WRITE") == 0){

		char* interface = getNextWord(operation, &offset);
		char* fileName = getNextWord(operation, &offset);
		char* physicalAddresChar = getNextWord(operation, &offset);
		uint32_t physicalAddress = getValue(physicalAddresChar);
		char* lengthChar = getNextWord(operation, &offset);
		uint32_t length = getValue(lengthChar);
		char* pointerChar = getNextWord(operation, &offset);
		uint32_t filePointer = getValue(pointerChar);
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_FS_WRITE_params* ioSend = NEW_IO_FS_WRITE(pid, fileName, physicalAddress, length, filePointer);
			send_FS_WRITE(ioSend, interfaceAndSocket->socket);
		} else {
			free(bufferIO);
			free(interface);
			free(physicalAddresChar);
			free(lengthChar);
			free(pointerChar);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
		}
		free(bufferIO);
		free(interface);
		free(physicalAddresChar);
		free(lengthChar);
		free(pointerChar);
	}
	if(strcmp(bufferIO, "IO_FS_READ") == 0){

		char* interface = getNextWord(operation, &offset);
		char* fileName = getNextWord(operation, &offset);
		char* physicalAddresChar = getNextWord(operation, &offset);
		uint32_t physicalAddress = getValue(physicalAddresChar);
		char* lengthChar = getNextWord(operation, &offset);
		uint32_t length = getValue(lengthChar);
		char* pointerChar = getNextWord(operation, &offset);
		uint32_t filePointer = getValue(pointerChar);
		io* interfaceAndSocket = isInterfaceConnected(interface);
		if (interfaceAndSocket != NULL) {
			IO_FS_READ_params* ioSend = NEW_IO_FS_READ(pid, fileName, physicalAddress, length, filePointer);
			send_FS_READ(ioSend, interfaceAndSocket->socket);
		} else {
			free(bufferIO);
			free(interface);
			free(physicalAddresChar);
			free(lengthChar);
			free(pointerChar);
			bool findPcb(void* elem) {
				return findPcbByPid(pid, elem);
			};
			pthread_mutex_lock(&sListBlockeds);
			pcb *pcbBloqued = list_find(listBlockeds, findPcb);
			list_remove_element(listBlockeds, pcbBloqued);
			pthread_mutex_unlock(&sListBlockeds);
			printf("Joder hostia, se ha eliminado un pcb!\n");
			deletePcb(pcbBloqued);
			pthread_exit(0);
		}
		free(bufferIO);
		free(interface);
		free(physicalAddresChar);
		free(lengthChar);
	}
	pthread_exit(0);
}

io* checkIfIos(int socket) {
	printf("VERIFICA?\n");
	char* interfaceName = recvMessage(socket);
	printf("Recibí el nombre de la interfaz: '%s'\n", interfaceName);
	io* newInterface = (io*)malloc(sizeof(io));  // Asignar memoria para la nueva estructura io
    if (newInterface == NULL) {
        perror("Error al asignar memoria para newInterface");
        return NULL;
    }
	newInterface->name = interfaceName;
	newInterface->socket = socket;
	if (nextFreeIndex < MAX_INTERFACES) {
		interfacesIoActivas[nextFreeIndex] = newInterface;
		nextFreeIndex++;
		send(socket, 0, sizeof(int32_t), 0);
	} else {
		printf("No hay más espacio en interfacesIoActivas para agregar otro cliente.\n");
		send(socket, -1, sizeof(int32_t), 0);
	}
	return newInterface;
}


io* isInterfaceConnected(char* interfaceName) {
    for (int i = 0; i < nextFreeIndex; i++) {  // nextFreeIndex indica el número actual de elementos en el array
        if (strcmp(interfacesIoActivas[i]->name, interfaceName) == 0 ) {
            log_info(logger, "La interfaz '%s' está conectada", interfaceName);
            return interfacesIoActivas[i];  // El cliente está en el array
        }
    }
    log_error(logger, "La interfaz NO está conectada");
    return NULL;  // El cliente no está en el array
}


void queue_iterate(t_queue* queue, void (*func)(void*)) {
    if (queue == NULL || func == NULL) {
        return; // Salir si la cola o la función son NULL
    }

    t_list* list = queue->elements;
    if (list == NULL) {
        return; // Salir si la lista en la cola es NULL
    }

    t_link_element* current = list->head;
    while (current != NULL) {
        func(current->data); // Aplicar la función al dato actual
        current = current->next; // Avanzar al siguiente elemento
    }
}

void stopPlanner() {
	if (plannerIsRunning) {
		printf("Planificación Detenida\n");
		plannerIsRunning = 0;
		sem_wait(&sCanPlan);
	} else {
		printf("El planificador ya se encuentra detenido \n");
	}
	
}

void runPlanner() {
	if (!plannerIsRunning) {
		printf("Planificación Iniciada\n");
		plannerIsRunning = 1;
		sem_post(&sCanPlan);
	} else {
		printf("El planificador ya se encuentra corriendo \n");
	}
}

void runChangeMultiprogramming(char* value) {
    pthread_t tidChangeMultiprogram;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // Hacer que el hilo sea joinable

    char* valueCopy = strdup(value); // Hacer una copia del valor para pasarlo al hilo
    if (pthread_create(&tidChangeMultiprogram, &attr, changeMultiprogramming, valueCopy) != 0) {
        perror("Error al crear el hilo");
        free(valueCopy); // Liberar la memoria si la creación del hilo falla
    } else {
        // Esperar a que el hilo termine antes de continuar
        pthread_join(tidChangeMultiprogram, NULL);
    }

    pthread_attr_destroy(&attr);
}

void changeMultiprogramming(char* value) {
	int newValue = getValue(value);
	int currentValue = getValue(GRADO_MULTIPROGRAMACION);
	if(newValue > 0) {
		if (newValue > currentValue) {
			for (int i = 0; i < newValue - currentValue; i++) {
				sem_post(&sQueueReadysMax);
			}
		} else if (newValue < currentValue) {
			for (int i = 0; i < currentValue - newValue; i++) {
				sem_wait(&sQueueReadysMax);
			}
		}
		strcpy(GRADO_MULTIPROGRAMACION, value);
		printf("Nuevo valor de Multiprogramacion: %i\n", getValue(GRADO_MULTIPROGRAMACION));
	} else {
		printf("Ingrese un número válido, mayor a 0\n");
	}
}

void allPcbStates() {
	char* states = malloc(sizeof(char)*1000);
	char* stateAux = malloc(sizeof(char)*100);
	strcpy(states, "Estado actual de todos los procesos:\n");
	if (currentyRunningPcb != NULL) {
		sprintf(stateAux, "PID: %u, Estado: %s\n", currentyRunningPcb->pid, pcbStateChar(currentyRunningPcb->state));
		strcat(states, stateAux);
	}

	void catData (void* data) {
		pcb* pcbData = (pcb*)data;
		sprintf(stateAux, "PID: %u, Estado: %s\n", pcbData->pid, pcbStateChar(pcbData->state));
		strcat(states, stateAux);
	}

	pthread_mutex_lock(&sQueueNews);
	if(!queue_is_empty(queueNews)) {
		queue_iterate(queueNews, catData);
	}
	pthread_mutex_unlock(&sQueueNews);

	pthread_mutex_lock(&sQueueReadys);
	if(!queue_is_empty(queueReadys)) {
		queue_iterate(queueReadys, catData);
	}
	pthread_mutex_unlock(&sQueueReadys);


	pthread_mutex_lock(&sQueueAux);
	if(!queue_is_empty(queueAux)) {
		queue_iterate(queueAux, catData);
	}
	pthread_mutex_unlock(&sQueueAux);


	pthread_mutex_lock(&sListBlockeds);
	if(!list_is_empty(listBlockeds)) {
		list_iterate(listBlockeds, catData);
	}
	pthread_mutex_unlock(&sListBlockeds);

	printf("%s", states);
	free(states);
	free(stateAux);
}

void finishProcess(uint32_t pid) {
    bool findPcb(void* elem) {
        return findPcbByPid(pid, elem);
    };

    pcb* pcbToFinish;

    if(currentyRunningPcb != NULL) {
		if(currentyRunningPcb->pid == pid) {
			printf("Se remueve proceso PID %u que estaba en estado %s\n", pid, pcbStateChar(currentyRunningPcb->state));
			interruptProcessCpu("CONSOLE");
			return;
		}
	}

    pthread_mutex_lock(&sQueueNews);
    pcbToFinish = list_find(queueNews->elements, findPcb);
    if(pcbToFinish != NULL) {
        printf("Se remueve proceso PID %u que estaba en estado %s\n", pid, pcbStateChar(pcbToFinish->state));
        list_remove_element(queueNews->elements, pcbToFinish);
        deletePcb(pcbToFinish);
        pthread_mutex_unlock(&sQueueNews);
        liberarRecursos(pid);
        return;
    }
    pthread_mutex_unlock(&sQueueNews);

    pthread_mutex_lock(&sQueueReadys);
    pcbToFinish = list_find(queueReadys->elements, findPcb);
    if(pcbToFinish != NULL) {
        printf("Se remueve proceso PID %u que estaba en estado %s\n", pid, pcbStateChar(pcbToFinish->state));
        list_remove_element(queueReadys->elements, pcbToFinish);
        deletePcb(pcbToFinish);
        pthread_mutex_unlock(&sQueueReadys);
        liberarRecursos(pid);
        return;
    }
    pthread_mutex_unlock(&sQueueReadys);

    pthread_mutex_lock(&sQueueAux);
    pcbToFinish = list_find(queueAux->elements, findPcb);
    if(pcbToFinish != NULL) {
        printf("Se remueve proceso PID %u que estaba en estado %s\n", pid, pcbStateChar(pcbToFinish->state));
        list_remove_element(queueAux->elements, pcbToFinish);
        deletePcb(pcbToFinish);
        pthread_mutex_unlock(&sQueueAux);
        liberarRecursos(pid);
        return;
    }
    pthread_mutex_unlock(&sQueueAux);

    pthread_mutex_lock(&sListBlockeds);
    pcbToFinish = list_find(listBlockeds, findPcb);
    if(pcbToFinish != NULL) {
        printf("Se remueve proceso PID %u que estaba en estado %s\n", pid, pcbStateChar(pcbToFinish->state));
        list_remove_element(listBlockeds, pcbToFinish);
        deletePcb(pcbToFinish);
        pthread_mutex_unlock(&sListBlockeds);
        liberarRecursos(pid);
        return;
    }
    pthread_mutex_unlock(&sListBlockeds);

    printf("El PID %u no esta en ejecución\n", pid);
}


void liberarRecursosDePID(Recursos_de_PID* recursosDePID) {
    if (recursosDePID != NULL) {
        list_destroy_and_destroy_elements(recursosDePID->recursos, free);
        free(recursosDePID);
    }
}

void agregarRecurso(Recursos_de_PID* recursosDePID, void* recurso) {
    list_add(recursosDePID->recursos, recurso);
};

Recursos_de_PID* crearRecursosDePID(int pid) {
    Recursos_de_PID* recursosDePID = buscarRecursosDePID(listResourcesPcbs, pid);
    if (recursosDePID != NULL) {
        // Si ya existe, retorna el existente
        return recursosDePID;
    }

    // Si no existe, crea uno nuevo
    recursosDePID = malloc(sizeof(Recursos_de_PID));
    if (recursosDePID == NULL) {
        perror("Unable to allocate memory");
        exit(EXIT_FAILURE);
    }
    recursosDePID->pid = pid;
    recursosDePID->recursos = list_create();
    if (recursosDePID->recursos == NULL) {
        perror("Unable to create list");
        free(recursosDePID);
        exit(EXIT_FAILURE);
    }
    list_add(listResourcesPcbs, recursosDePID);
    return recursosDePID;
}


void removerRecurso(Recursos_de_PID* recursosDePID, const char* recurso) {
    bool matchRecurso(void* elem) {
        return strcmp((char*)elem, recurso) == 0;
    }

    char* recursoEncontrado = list_remove_by_condition(recursosDePID->recursos, matchRecurso);
  
}

Recursos_de_PID* buscarRecursosDePID(t_list* listResourcesPcbs, int pid) {
    bool matchPid(void* elem) {
        Recursos_de_PID* recursosParaENcontrar = (Recursos_de_PID*)elem;
        return recursosParaENcontrar->pid == pid;
    }
    return list_find(listResourcesPcbs, matchPid);
}


void printRecursosDePID(Recursos_de_PID* recursosDePID) {
    if (recursosDePID == NULL) {
        printf("Recursos_de_PID is NULL\n");
        return;
    }

    printf("PID: %d\n", recursosDePID->pid);
    printf("Recursos:\n");

    void printRecurso(void* elem) {
        char* recurso = (char*)elem;
        printf("  %s\n", recurso);
    }

    list_iterate(recursosDePID->recursos, printRecurso);
}



void liberarRecursos(uint32_t pid) {
    // Buscar los recursos asociados al PID
    Recursos_de_PID* recursosDePID = buscarRecursosDePID(listResourcesPcbs, pid);
    if (recursosDePID != NULL) {
        // Iterar sobre cada recurso en recursosDePID->recursos
        void liberarRecurso(void* elem) {
            char* resource = (char*)elem;

            // Remover el recurso de recursosDePID
            removerRecurso(recursosDePID, resource);

            // Encontrar el índice del recurso en el array de recursos
            int index = findIndexOfString(recursosArray, recursosSize, resource);

            if (index != -1) {
                // Incrementar la instancia del recurso
				recursos[index].instancia++;
                printf("El recurso %s tiene %d instancias.\n", resource, recursos[index].instancia);

            } else {
                printf("El recurso %s no se encuentra en el array.\n", resource);
            }

            // Intentar desbloquear procesos que estaban esperando este recurso
            bool findPcb(void* elem) {
                return findPcbByContext(resource, elem);
            }

            pthread_mutex_lock(&sListBlockeds);
            pcb *pcbBloqued = list_find(listBlockeds, findPcb);
            if (pcbBloqued == NULL) {
                printf("PCB not found in blocked list\n");
                pthread_mutex_unlock(&sListBlockeds);
            } else {
                pthread_mutex_unlock(&sListBlockeds);
                desblockPcb(findPcb);
            }
        }

        // Iterar sobre la lista de recursos
        list_iterate(recursosDePID->recursos, liberarRecurso);

        // Después de liberar todos los recursos, puedes limpiar la lista si es necesario
        list_clean_and_destroy_elements(recursosDePID->recursos, free);
		return;
    } else {
        printf("EL PROCESO NO TENIA RECURSOS %u\n", pid);
        return;
    }
}