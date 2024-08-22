#include "interactive_console.h"

// EJECUTAR_SCRIPT "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/c-comenta-pruebas/scripts_kernel/PRUEBA_PLANI3"
// EJECUTAR_SCRIPT "/home/utnso/Desktop/tp-2024-1c-RonaldinhOS/c-comenta-pruebas/scripts_kernel/PRUEBA_DEADLOCK2"

interpetAndExecCommand(t_queue* wordList, char* absolutePath) {
	// while (!queue_is_empty(wordList)) {
	// 	char* word = (char*)queue_pop(wordList);
	// 	printf("%s\n", word);
	// }
	char* function = toUpperCase(queue_pop(wordList));
	if (strcmp(function, "EJECUTAR_SCRIPT") == 0) {
		if(queue_size(wordList) == 1){
			char* path = queue_pop(wordList);
			removeLeadingSlash(&path);
			char* allPath = malloc(sizeof(char)*200);
			sprintf(allPath , "%s%s", absolutePath, path);
			printf("Ejecutamos un nuevo script ubicado en %s\n", allPath);
			FILE *file;
			int sizeInstruction = 150;
			char* instruction = malloc(sizeof(char) * sizeInstruction);

			file = fopen(allPath, "r");
			if (file == NULL) {
				perror("Error al abrir el archivo");
				return EXIT_FAILURE;
			}

			const char delimiter[] = " ";
			while (fgets(instruction, sizeInstruction, file) != NULL) {
				// Procesar la línea leída
				t_queue* wordList = splitString(instruction, delimiter);
				interpetAndExecCommand(wordList, absolutePath);
				queue_destroy_and_destroy_elements(wordList, free);
			}

			free(allPath);
			fclose(file);
			free(instruction);
		} else {
			printParameterScopeError();
			printf("Sintaxis: EJECUTAR_SCRIPT <PATH>\n");
			queue_clean(wordList);
		}

	} else if (strcmp(function, "INICIAR_PROCESO") == 0) {
		
		if(queue_size(wordList) == 1){
			char* path = queue_pop(wordList);
			removeLeadingSlash(&path);
			char* allPath = malloc(sizeof(char)*200);
			sprintf(allPath , "%s%s", absolutePath, path);
			if (access(allPath, F_OK) == 0) {
				printf("Iniciando proceso path: %s\n", allPath);
				pcb *newPcb = createPcb(allPath);
			} else {
				printf("El archivo no existe, path: %s\n", allPath);
			}
			free(allPath);
		} else {
			printParameterScopeError();
			printf("Sintaxis: INICIAR_PROCESO <PID>\n");
			queue_clean(wordList);
		}
	} else if (strcmp(function, "FINALIZAR_PROCESO") == 0) {
		if(queue_size(wordList) == 1){
			char* pidStr = queue_pop(wordList);
			finishProcess(getValue(pidStr));
		} else {
			printParameterScopeError();
			printf("Sintaxis: FINALIZAR_PROCESO <PID>\n");
			queue_clean(wordList);
		}

	} else if (strcmp(function, "DETENER_PLANIFICACION") == 0) {
		if(queue_size(wordList) == 0){
			stopPlanner();
		} else {
			printParameterScopeError();
			printf("Sintaxis: DETENER_PLANIFICACION\n");
			queue_clean(wordList);
		}

	} else if (strcmp(function, "INICIAR_PLANIFICACION") == 0) {
		if(queue_size(wordList) == 0){
			runPlanner();
		} else {
			printParameterScopeError();
			printf("Sintaxis: INICIAR_PLANIFICACION\n");
			queue_clean(wordList);
		}

	} else if (strcmp(function, "MULTIPROGRAMACION") == 0) {
		if(queue_size(wordList) == 1){
			char* valueStr = queue_pop(wordList);
			runChangeMultiprogramming(valueStr);
		} else {
			printParameterScopeError();
			printf("Sintaxis: MULTIPROGRAMACION <VALOR>\n");
			queue_clean(wordList);
		}

	} else if (strcmp(function, "PROCESO_ESTADO") == 0) {
		if(queue_size(wordList) == 0){
			allPcbStates();
		} else {
			printParameterScopeError();
			printf("Sintaxis: PROCESO_ESTADO\n");
			queue_clean(wordList);
		}

	} else {
		printf("Funcion desconocida\n");
	}
	free(function);
}

void printParameterScopeError() {
	printf("La cantidad de parametros no corresponde a la funcion a ejecutar\n");
}

char* toUpperCase(char* str) {
    // Verificamos si el puntero es nulo
    if (str == NULL) {
        printf("Error: Puntero nulo.\n");
        return NULL;
    }

    // Obtenemos la longitud de la cadena
    size_t len = strlen(str);

    // Creamos un nuevo puntero para almacenar la cadena convertida a mayúsculas
    char* upperStr = (char*)malloc((len + 1) * sizeof(char)); // +1 para el carácter nulo al final
    if (upperStr == NULL) {
        printf("Error: No se pudo asignar memoria.\n");
        return NULL;
    }

    // Convertimos cada carácter a mayúsculas y lo copiamos al nuevo puntero
    for (size_t i = 0; i < len; i++) {
        upperStr[i] = toupper(str[i]);
    }
    upperStr[len] = '\0'; // Agregamos el carácter nulo al final de la cadena

    // Liberamos la memoria del puntero original
    free(str);

    return upperStr;
}

int endsWithSlash(char *str) {
    // Verificar si la cadena no está vacía y termina con '/'
    return (str != NULL && str[strlen(str) - 1] == '/');
}

void removeLeadingSlash(char** str) {
    if (*str[0] == '/') {
        (*str)++;
    }
}