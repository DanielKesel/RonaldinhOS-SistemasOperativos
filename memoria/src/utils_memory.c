#include "utils_memory.h"
#include <unistd.h>


void inicializarConfig() {

	memoryConfig = initConfig("../memory.config");
    memoryPort = getConfigValue(memoryConfig , "PUERTO_ESCUCHA");
    lengthMemory = getConfigValue(memoryConfig , "TAM_MEMORIA");
    lengthPage = getConfigValue(memoryConfig , "TAM_PAGINA");
    pathInstructions = getConfigValue(memoryConfig , "PATH_INSTRUCCIONES");
    answerDelay = getConfigValue(memoryConfig , "RETARDO_RESPUESTA");
    log_info(log_memory, "Variable: \"%s\" has been assigned value: \"%s\" from the config file", STRINGIFY(memoryPort), memoryPort);
    log_info(log_memory, "Variable: \"%s\" has been assigned value: \"%s\" from the config file", STRINGIFY(lengthMemory), lengthMemory);
    log_info(log_memory, "Variable: \"%s\" has been assigned value: \"%s\" from the config file", STRINGIFY(lengthPage), lengthPage);
    log_info(log_memory, "Variable: \"%s\" has been assigned value: \"%s\" from the config file", STRINGIFY(pathInstructions), pathInstructions);
    log_info(log_memory, "Variable: \"%s\" has been assigned value: \"%s\" from the config file", STRINGIFY(answerDelay), answerDelay);
    
}

// void reciveOfKernel(kernelClientFDSocket){
//     t_list* lista;
//     while (1) {
//         int cod_op;
//     //Recibir la operación del cliente
//     cod_op = recvOperation(kernelClientFDSocket);
//     switch (cod_op) {
//         case NEW_PROCESS:
//             //process thisIsTheNewProcess = recvFromKernel(kernelClientFDSocket);
//             break;
//         case -1:
//             log_error(logMemoryString , "The client disconnected");
//             break;
//         default:
//             log_warning(logMemoryString , "Unknown operation");
//             break;
//         }
//         if (cod_op == -1) {
//             break; // Salir del bucle while si hay un error
//         }
//     }
// }
int bitsToBytes(int bits){
	int bytes;
	if(bits < 8)
		bytes = 1; 
	else
	{
		double c = (double) bits;
		bytes = ceil(c/8.0);
	}
	
	return bytes;
}
char* recvFromKernel(int socketClient)
{
	int size;
	char* buffer = recvBuffer(&size, socketClient);
    return buffer;
	//log_info(log_memory, "Message recived: %s", buffer);
	// acordarse de hacer el free fuera de la funcion free(buffer);
}


//Función para guardar las instrucciones del pseudocodigo en un array de punteros a char.
char** appetiteForInstruction(char* path) {
    FILE *archivo;
    char linea[MAX_LINE_LENGTH]; // Variable para almacenar temporalmente cada línea leída
    char **lineas = NULL; // Array de punteros para almacenar las líneas
    int contador = 0; // Contador de líneas

    if (access(path, F_OK) == 0) {
        log_info(log_memory,"En la dirección indicada existe el archivo de pseudocodigo de instrucciones");
    } else {
        log_error(log_memory, "No existe el archivo pseudocodigo de instrucciones");
        return NULL;
    }

    // Abre el archivo en modo lectura
    archivo = fopen(path, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }
    log_info(log_memory,"Archivo de pseudocodigo leído correctamente");
    // Bucle para contar las líneas del archivo
    while (fgets(linea, MAX_LINE_LENGTH, archivo) != NULL) {
        contador++;
    }

    // Vuelve a colocar el puntero de archivo al inicio del archivo
    fseek(archivo, 0, SEEK_SET);

    // Asigna memoria para el array de punteros
    lineas = (char **)malloc((contador + 1) * sizeof(char *)); // +1 para el último elemento NULL

    // Reinicia el contador
    contador = 0;

    // Bucle para leer cada línea del archivo y almacenarla en el array
    while (fgets(linea, MAX_LINE_LENGTH, archivo) != NULL) {
        // Asigna memoria para almacenar la línea actual
        lineas[contador] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
        // Copia la línea leída en el array
        strcpy(lineas[contador], linea);
        contador++; // Incrementa el contador
    }

    // Cierra el archivo
    fclose(archivo);

    // Marca el final del array con un puntero nulo
    lineas[contador] = NULL;

    return lineas;
}

void readArrayOfInstructions(char** arrayOfInstructions)
{

     if (arrayOfInstructions != NULL) {
        //printf("Contenido del archivo:\n");

        //Itera a través del array de instrucciones
        for (int i = 0; arrayOfInstructions[i] != NULL; i++) {
            //printf("Línea %d: %s", i + 1, arrayOfInstructions[i]);
            // Libera la memoria de cada línea
            //free(arrayOfInstructions[i]);  // VER SI SOLUCIONA MEMORY LEAK 6496 BYTES
        }

        //Libera la memoria del array de punteros
        //free(arrayOfInstructions);  // VER SI SOLUCIONA MEMORY LEAK 6496 BYTES
    } else {
        log_error(log_memory, "Error al leer las instrucciones del archivo.\n");
        }

}

int waitRequestOfCPU(int* value, int socketClient)
{
    // Configurar el socket para modo bloqueante
    fcntl(socketClient, F_SETFL, 0);

    int32_t resultOk = 0;

    // Esperar hasta que haya datos disponibles para leer en el socket
    int bytes_leidos = recv(socketClient, value, sizeof(int), MSG_WAITALL);
    if (bytes_leidos != sizeof(int)) {
        // Error al leer el entero
        perror("recv");
        log_info(log_memory, "waitRequestOfCPU - FAIL: Error al recibir el entero");
        return -1; // Código de error personalizable
    }
    
    // Mostrar el valor recibido por el cliente
    //printf("Valor recibido por el cliente: %d\n", *value);

    // Enviar una confirmación al cliente
    bytes_leidos = send(socketClient, &resultOk, sizeof(int32_t), 0);
    if (bytes_leidos != sizeof(int32_t)) {
        // Error al enviar la confirmación
        perror("send");
        log_info(log_memory, "waitRequestOfCPU - FAIL: Error al enviar la confirmación");
        return -1; // Código de error personalizable
    }

    log_info(log_memory, "waitRequestOfCPU - OK");

    // Todo ha ido bien
    return 0;
}

int waitProgramCounterStrike(int* value, int socketClient)
{
    int32_t resultOk = 0;
    int32_t received_value;

    // Recibir el valor del program counter del cliente
    int bytes_leidos = recv(socketClient, &received_value, sizeof(int32_t), MSG_WAITALL); 

    // Verificar si se recibió la cantidad correcta de bytes
    if (bytes_leidos != sizeof(int32_t)) {
        log_info(log_memory, "waitProgramCounterStrike - FAIL");
        return -1; // Código de error personalizable
    }

    // Asignar el valor recibido al puntero value
    *value = received_value;

    // Enviar la confirmación de que se recibió el valor correctamente
    bytes_leidos = send(socketClient, &resultOk, sizeof(int32_t), 0);
    if (bytes_leidos != sizeof(int32_t)) {
        log_error(log_memory, "Error al enviar la confirmación de recepción del valor");
        return -1; // Código de error personalizable
    }

    log_info(log_memory, "waitProgramCounterStrike - Ok");
    return 0;
}

int sendInstructionToCpu(const char *buffer, int socket) {
    // Enviar el mensaje
    sendLine(buffer, socket);

    int32_t result;

    // Recibir el resultado
    ssize_t bytesReceived = recv(socket, &result, sizeof(int32_t), MSG_WAITALL);
    if (bytesReceived <= 0) {
        log_error(log_memory, "sendInstructionToCpu - Error al recibir el resultado");
        return -1; // Devolver un código de error
    }

    // Comprobar el resultado
    if (result == 0) {
        log_info(log_memory, "sendInstructionToCpu - Ok");
    } else {
        log_info(log_memory, "sendInstructionToCpu - FAIL");
    }

    return result; // Devolver el resultado de la operación
}

void logNewPageTable(int pid, int size) {
    logMemoryString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	char* sizeStr = intToChar(size);
	strcpy(logMemoryString, "PID: ");
	strcat(logMemoryString, pidStr);
	strcat(logMemoryString, " - Acción: CREAR - Tamaño: ");
	strcat(logMemoryString, sizeStr);
	log_info(log_memory, logMemoryString);
	free(logMemoryString);
	free(pidStr);
	free(sizeStr);
}

void logDestroyPageTable(int pid, int size) {
    logMemoryString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	char* sizeStr = intToChar(size);
	strcpy(logMemoryString, "PID: ");
	strcat(logMemoryString, pidStr);
	strcat(logMemoryString, " - Acción: DESTRUIR - Tamaño: ");
	strcat(logMemoryString, sizeStr);
	log_info(log_memory, logMemoryString);
	free(logMemoryString);
	free(pidStr);
	free(sizeStr);
}

void logPageTableAccess(int pid, int page, int marco) {
    logMemoryString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	char* pageStr = intToChar(page);
	char* marcoStr = intToChar(marco);
	strcpy(logMemoryString, "PID: ");
	strcat(logMemoryString, pidStr);
	strcat(logMemoryString, " - Pagina: ");
	strcat(logMemoryString, pageStr);
    strcat(logMemoryString, " - Marco: ");
	strcat(logMemoryString, marcoStr);
	log_info(log_memory, logMemoryString);
	free(logMemoryString);
	free(pidStr);
	free(pageStr);
	free(marcoStr);
}

void logProcessExpansion(int pid, int actualSize, int newSize) {
    logMemoryString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	char* actualSizeStr = intToChar(actualSize);
	char* newSizeStr = intToChar(newSize);
	strcpy(logMemoryString, "PID: ");
	strcat(logMemoryString, pidStr);
	strcat(logMemoryString, " - Tamaño Actual: ");
	strcat(logMemoryString, actualSizeStr);
    strcat(logMemoryString, " - Tamaño a Ampliar: ");
	strcat(logMemoryString, newSizeStr);
	log_info(log_memory, logMemoryString);
	free(logMemoryString);
	free(pidStr);
	free(actualSizeStr);
	free(newSizeStr);
}

void logProcessReduction(int pid, int actualSize, int newSize) {
    logMemoryString = malloc(sizeof(char) *50);
	char* pidStr = intToChar(pid);
	char* actualSizeStr = intToChar(actualSize);
	char* newSizeStr = intToChar(newSize);
	strcpy(logMemoryString, "PID: ");
	strcat(logMemoryString, pidStr);
	strcat(logMemoryString, " - Tamaño Actual: ");
	strcat(logMemoryString, actualSizeStr);
    strcat(logMemoryString, " - Tamaño a Reducir: ");
	strcat(logMemoryString, newSizeStr);
	log_info(log_memory, logMemoryString);
	free(logMemoryString);
	free(pidStr);
	free(actualSizeStr);
	free(newSizeStr);
}

void logAccessToUserSpaceRead(int pid, int dir, char* size) {
    char* logMemoryString;
    char* pidStr = intToChar(pid);
    char* dirStr = intToChar(dir);


    int valorConvertido = (int)size;

    // Convertir 'size' (que es un char*) a entero
    char caracter = size[0];  // Tomar el primer carácter de 'size'
    valorConvertido = (int)caracter;  // Convertir el carácter a su valor entero


    // Asignar memoria para logMemoryString
    logMemoryString = (char*)malloc(sizeof(char) * 150);
    if (logMemoryString == NULL) {
        fprintf(stderr, "Error de asignación de memoria\n");
        free(pidStr);
        free(dirStr);
        exit(EXIT_FAILURE);
    }

    // Crear la cadena de log
    sprintf(logMemoryString, "PID: %s - Accion: LEER - Direccion fisica: %s - VALOR %d", pidStr, dirStr, valorConvertido);

    // Log de información
    log_info(log_memory, logMemoryString);
    log_info(log_memory, "EQUIVALENTE EN ASCII: %s", size);

    // Liberar memoria
    free(logMemoryString);
    free(pidStr);
    free(dirStr);
}

void logAccessToUserSpaceWrite(int pid, int dir, char* size) {
    char* logMemoryString;
    char* pidStr = intToChar(pid);
    char* dirStr = intToChar(dir);
    
    int valorConvertido;

    // Convertir 'size' (que es un char*) a entero
    char caracter = size[0];  // Tomar el primer carácter de 'size'
    valorConvertido = (int)caracter;  // Convertir el carácter a su valor entero

    // Asignar memoria para logMemoryString
    logMemoryString = (char*)malloc(sizeof(char) * 150);
    if (logMemoryString == NULL) {
        fprintf(stderr, "Error de asignación de memoria\n");
        free(pidStr);
        free(dirStr);
        exit(EXIT_FAILURE);
    }

    // Crear la cadena de log
    sprintf(logMemoryString, "PID: %s - Accion: ESCRIBIR - Direccion fisica: %s - VALOR %d", pidStr, dirStr, valorConvertido);

    // Log de información
    log_info(log_memory, logMemoryString);
    log_info(log_memory, "EQUIVALENTE EN ASCII: %s", size);

    // Liberar memoria
    free(logMemoryString);
    free(pidStr);
    free(dirStr);
}
