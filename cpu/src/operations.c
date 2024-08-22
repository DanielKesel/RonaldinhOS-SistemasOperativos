#include "operations.h"

pcb* CurrentPCB;

int executeOperations(char* operation, pcb* PCB){

    int interruptPcb = 0; // Segun la operación, si hace falta interrumpir el PCB (por IO por ejemplo), tiene que estar en 1, sino, en 0

    CurrentPCB = PCB;

    char* buffer;                                   //Buffer para guardar lo que lee getNextWord()
    int offset = 0;

    buffer = getNextWord(operation, &offset);
    if (strcmp(buffer, "SET") == 0) {
        //printf("Se reconoce la operación SET. \n");
        char* registerName = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", registerName);
        char* valueChar = getNextWord(operation, &offset);
        int value = getValue(valueChar);
        //printf("Se leyó el valor: %d. \n", value);

        SET(registerName, value);
        void* registerValueAddress = getRegister(registerName);
         //printf("Valor de registerValueAddress antes del casteo: %p\n", registerValueAddress);
        
        if (strlen(registerName) == 3 || !strcmp(registerName, "SI") || !strcmp(registerName, "DI") || !strcmp(registerName, "PC")) {
            uint32_t length = *(uint32_t*)registerValueAddress;
            // Imprime el valor de length después del casteo
            //printf("Contenido como uint32_t: %u\n", length);
        } else {
            uint8_t length = *(uint8_t*)registerValueAddress;
            // Imprime el valor de length después del casteo
            //printf("Contenido como uint8_t: %u\n", length);
        }
        free(valueChar);  // VER SI SOLUCIONA MEMORY LEAK 2250 BYTES
        free(buffer);
        free(registerName);

    } else if (strcmp(buffer, "MOV_IN") == 0) {
        //printf("Se reconoce la operacion MOV_IN. \n");
        char* dest = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", dest);
        char* src = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", src);

        MOV_IN(dest, src);
        free(buffer);  // VER SI SOLUCIONA MEMORY LEAK 2250 BYTES
        free(dest);
        free(src);
    } else if (strcmp(buffer, "MOV_OUT") == 0) {
        //printf("Se reconoce la operacion MOV_OUT. \n");
        char* src = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", src);
        char* dest = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", dest);
        
        MOV_OUT(src, dest);
        free(buffer);  // VER SI SOLUCIONA MEMORY LEAK 2250 BYTES
        free(src);
        free(dest);
    } else if (strcmp(buffer, "SUM") == 0) {
        //printf("Se reconoce la operación SUM. \n");
        char* destinyBuffer = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", destinyBuffer);
        char* originBUffer = getNextWord(operation, &offset);;
        //printf("Se leyó el registro %s. \n", originBUffer );

        SUM(destinyBuffer, originBUffer);
        free(buffer);
        free(destinyBuffer);
        free(originBUffer);
    } else if (strcmp(buffer, "SUB") == 0) {
        //printf("Se reconoce la operación SUB. \n");
        char* destinyBuffer = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", destinyBuffer );
        char* originBUffer = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", originBUffer);

        SUB(destinyBuffer, originBUffer);

        free(buffer);
        free(destinyBuffer);
        free(originBUffer);
    
    } else if (strcmp(buffer, "JNZ") == 0) {
        //printf("Se reconoce la operación JNZ. \n");
        char* registerName = getNextWord(operation, &offset);
        //printf("Se leyó el registro %s. \n", registerName);
        char* valueChar = getNextWord(operation, &offset);
        int instruction = getValue(valueChar);
        //printf("Se leyó el valor de instrucción: %d. \n", instruction);

        JNZ(registerName, instruction);

        free(buffer);
        free(valueChar);
        free(registerName);

    } else if (strcmp(buffer, "RESIZE") == 0) {

        char* lengthChar = getNextWord(operation, &offset);
        int length = getValue(lengthChar);

        //printf("Le mando a memoria el tamanio de proceso\n");
        
        RESIZE_params* tamanioAEnviar = NEW_RESIZE(CurrentPCB->pid, length);
        send_RESIZE(tamanioAEnviar, memoryConnection);
        int okey;
        recvInt(&okey, memoryConnection);
        if(!okey) {
            interruptPcb = 1;
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR RESIZE: Out Of Memory");
        }
        free(tamanioAEnviar);  // VER SI SOLUCIONA MEMORY LEAK 48 BYTES
        free(lengthChar);  // VER SI SOLUCIONA MEMORY LEAK 300 BYTES
        free(buffer);  // VER SI SOLUCIONA MEMORY LEAK 2250 BYTES

    }  else if (strcmp(buffer, "COPY_STRING") == 0) {
        //printf("Se reconoce la operacion COPY_STRING\n");
        char* tamanio = getNextWord(operation, &offset);
        void* registroSI = getRegister("SI");
        char* registroSIchar = (char*)registroSI;
        void* registroDI = getRegister("DI");
        uint32_t logicalAddressEscribir = *(uint32_t*)registroDI;
        uint32_t logicalAddressLeer = *(uint32_t*)registroSI;
        uint32_t direccionFIsicaDESTINO = logicalToPhysicalAddress(logicalAddressEscribir, CurrentPCB->pid);
        uint32_t direccionFIsicaORIGEN = logicalToPhysicalAddress(logicalAddressLeer, CurrentPCB->pid);

        COPY_STRING_params* stringAEnviar = NEW_COPY_STRING(CurrentPCB->pid, direccionFIsicaDESTINO, direccionFIsicaORIGEN, getValue(tamanio));
        send_COPY_STRING(stringAEnviar, memoryConnection);

        // Liberar la memoria asignada
        free(tamanio);
        free(buffer);
 

    } else if (strcmp(buffer, "WAIT") == 0) {
            
            char* resource = getNextWord(operation, &offset);
            uint32_t result;
            sendResourceWAIT(resource, kernelSocket);
            sendInt(CurrentPCB->pid, kernelSocket);

            log_info(log_cpu, "WAIT SEMAFORO\n");
            sem_wait(&sExecuteCodeOpResources);
            switch (cod_op_resource) {
                case CONTINUE:
                    log_info(log_cpu, "CONTINUE\n");
                    break;
                case BLOCK:
                    log_info(log_cpu, "BLOCK\n");
                    interruptPcb = 1;
                    strcpy(PCB->context->reason, "WAIT");
                    strcpy(PCB->context->info, resource);
                    log_info(log_cpu, "{WAIT} Este es el resultado final: %s\n", resource);
                    break;
                case EMPTY_INSTANCE:
                    log_info(log_cpu, "EMPTY_INSTANCE\n");
                    interruptPcb = 1;
                    strcpy(PCB->context->reason, "EXIT");
                    strcpy(PCB->context->info, "");
                    break;
                case -1:
                    exit(EXIT_FAILURE);
                    break;
                default:
                    log_info(log_cpu, "DEFAULT XD\n");
                    log_warning(logger, "Server: Unknown operation");
                    break;
            }
            free(buffer);
            free(resource);
    }   else if (strcmp(buffer, "SIGNAL") == 0) {
            
            char* resource = getNextWord(operation, &offset);
            log_info(log_cpu, "SIGNAL SEMAFORO\n");
            sendResourceSIGNAL(resource, kernelSocket);
            sendInt(CurrentPCB->pid, kernelSocket);
            sem_wait(&sExecuteCodeOpResources);
            switch (cod_op_resource) {
                case CONTINUE:
                    log_info(log_cpu, "CONTINUE\n");
                    break;
                case EMPTY_INSTANCE:
                    log_info(log_cpu, "EMPTY_INSTANCE\n");
                    interruptPcb = 1;
                    strcpy(PCB->context->reason, "EXIT");
                    strcpy(PCB->context->info, "");
                    break;
                case -1:
                    exit(EXIT_FAILURE);
                    break;
                default:
                    log_warning(log, "Server: Unknown operation");
                    break;
            }
            free(buffer);
            free(resource);
    } else if (strcmp(buffer, "IO_GEN_SLEEP") == 0) {
        interruptPcb = 1;
        char* iOName  = getNextWord(operation, &offset);
        char* timeToSleep = getNextWord(operation, &offset);

        // Concatenar las palabras leídas en un solo string
        char* result = malloc(sizeof(char) * 50); // 3 para los espacios y el terminador nulo
        if (result == NULL) {
            fprintf(stderr, "Error al asignar memoria\n");
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR IO_GEN_SLEEP: Error allocating memory");
            return interruptPcb;
        }
        strcpy(result, "IO_GEN_SLEEP ");
        strcat(result, iOName);
        strcat(result, " "); // Añadir un espacio entre iOName y timeToSleep
        strcat(result, timeToSleep); // Añadir timeToSleep al final de result

        //IO_GEN_SLEEP(result);
        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, result);
        log_info(log_cpu, "{IO_GEN_SLEEP} Este es el resultado final: %s\n", result);

        free(buffer);
        free(result);
        free(iOName);
        free(timeToSleep);

    }  else if (strcmp(buffer, "IO_STDIN_READ") == 0) {

        interruptPcb = 1;
        //printf("Se reconoce la operación IO_STDIN_READ. \n");
        char* interphaceName = getNextWord(operation, &offset);
        char* registerName = getNextWord(operation, &offset);
        void* registerValueAddress = getRegister(registerName);
        uint32_t logicalAddress = 0;
        // Determinar si el registro es de 8 o 32 bits
        if (strlen(registerName) == 3 || 
            !strcmp(registerName, "SI") || 
            !strcmp(registerName, "DI") || 
            !strcmp(registerName, "PC") ||
            !strcmp(registerName, "EAX") ||
            !strcmp(registerName, "EBX") ||
            !strcmp(registerName, "ECX") ||
            !strcmp(registerName, "EDX")) {
            logicalAddress = *(uint32_t*)registerValueAddress;
        } else {
            logicalAddress = *(uint8_t*)registerValueAddress;
        }

        //printf("Se leyó el registro %s, con valor %d\n", registerName, logicalAddress);

        // Leer nombre del segundo registro
        char* secondRegisterName  = getNextWord(operation, &offset);
        void* registerValueAddress2 = getRegister(secondRegisterName);
        uint32_t lengthRegister = 0;

        // Determinar si el segundo registro es de 8 o 32 bits
        if (strlen(secondRegisterName) == 3 || 
            !strcmp(secondRegisterName, "SI") || 
            !strcmp(secondRegisterName, "DI") || 
            !strcmp(secondRegisterName, "PC") ||
            !strcmp(secondRegisterName, "EAX") ||
            !strcmp(secondRegisterName, "EBX") ||
            !strcmp(secondRegisterName, "ECX") ||
            !strcmp(secondRegisterName, "EDX")) {
            lengthRegister = *(uint32_t*)registerValueAddress2;
        } else {
            lengthRegister = *(uint8_t*)registerValueAddress2;
        }
        //printf("El tamaño a leer de STDIN es:%d\n", lengthRegister);
        
        uint32_t physicalAddress = logicalToPhysicalAddress(logicalAddress, CurrentPCB->pid);

         // Determinar el tamaño del buffer necesario para la cadena resultante


        char* concat = malloc(sizeof(char)*50);
        if (concat == NULL) {
            log_error(log_cpu, "Unable to allocate memory");
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR IO_STDIN_READ: Error allocating memory");
            return interruptPcb;
        }

        // Formatear la cadena en el buffer
        strcpy(concat, "IO_STDIN_READ ");
        strcat(concat, interphaceName);
        sprintf(concat + strlen(concat), " %d", physicalAddress);
        sprintf(concat + strlen(concat), " %d", lengthRegister);


        int pid = (int)CurrentPCB->pid;
        char* dir = uint32ToChar(physicalAddress);      
        /* char* value = uint32ToChar();                               //NECESITAMOS CONOCER EL VALOR A ESCRIBIR/LEER ANTES DE ENVIARLO A IO O LO PODEMOS CONOCER DESPUES?
        double cantLogs = ceil(lengthRegister/ (float)pagesLength); 
        if( cantLogs <= 1 ){
            logWriteMemory(pid, dir, value);
        } else {
            for (i = 0; i < cantLogs; i++){
                physicalAddress += pagesLength;
                dir = uint32ToChar(physicalAddress);
                value = uint32ToChar();                             //NECESITAMOS CONOCER EL VALOR A ESCRIBIR/LEER ANTES DE ENVIARLO A IO O LO PODEMOS CONOCER DESPUES?
                logWriteMemory(pid, dir, value);
            }
            
        } */
        free(dir);
        //free(value);

        //IO_STDIN_READ_FN(concat);
        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, concat);

        // Imprimir la cadena resultante
        //printf("{STDIN} Este es el resultado final que se envía a kernel: %s\n", concat);

        // Liberar la memoria asignada
        free(concat);
        free(buffer);
        free(secondRegisterName);
        free(interphaceName);
        free(registerName);

    } else if (strcmp(buffer, "IO_STDOUT_WRITE") == 0) {
        interruptPcb = 1;
        //printf("Se reconoce la operación IO_STDOUT_WRITE. \n");
        char* interphaceName = getNextWord(operation, &offset);
        char* registerName = getNextWord(operation, &offset);
        
        //ESE NO SE LIBERA PORQUE TIENE LA MISMA DIRECCION DE MEMORIA QUE EL REGISTRO DE LA CPU
        void* registerValueAddress = getRegister(registerName); 

        uint32_t logicalAddress = 0;

        if (registerValueAddress != NULL) {
            if (strlen(registerName) == 3 || 
            !strcmp(registerName, "SI") || 
            !strcmp(registerName, "DI") || 
            !strcmp(registerName, "PC") ||
            !strcmp(registerName, "EAX") ||
            !strcmp(registerName, "EBX") ||
            !strcmp(registerName, "ECX") ||
            !strcmp(registerName, "EDX")) {
            logicalAddress  = *(uint32_t*)registerValueAddress;
            } else {
                logicalAddress = *(uint8_t*)registerValueAddress;
            }
            //printf("Se leyó el registro %s, con valor %u\n", registerName, logicalAddress);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", registerName);
        }
        char* secondRegisterName = getNextWord(operation, &offset); // Leer el nombre del registro

        uint32_t lengthRegister = 0; // Declaración fuera del if

        //ESE NO SE LIBERA PORQUE TIENE LA MISMA DIRECCION DE MEMORIA QUE EL REGISTRO DE LA CPU
        void* registerValueLength = getRegister(secondRegisterName); // Obtener el puntero devuelto por getRegister

        if (registerValueLength != NULL) {
            if (strlen(secondRegisterName) == 3 || 
            !strcmp(secondRegisterName, "SI") || 
            !strcmp(secondRegisterName, "DI") || 
            !strcmp(secondRegisterName, "PC") ||
            !strcmp(secondRegisterName, "EDX")) {
                lengthRegister  = *(uint32_t*)registerValueLength;
            } else {
                lengthRegister = *(uint8_t*)registerValueLength;
            }
            //printf("Se leyó el registro %s, con longitud %u\n", secondRegisterName, lengthRegister);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", secondRegisterName);
        }

        //printf("El tamaño a leer de STDOUT es:%d\n", lengthRegister);
       

        uint32_t physicalAddress = logicalToPhysicalAddress(logicalAddress, CurrentPCB->pid);

    

        // Asignar memoria para el buffer
        char* concat = malloc(sizeof(char)*50);
        if (concat == NULL) {
            log_error(log_cpu, "Unable to allocate memory");
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR IO_STDOUT_WRITE: Error allocating memory");
            return 1;
        }

        // Formatear la cadena en el buffer
        strcpy(concat, "IO_STDOUT_WRITE ");
        strcat(concat, interphaceName);
        sprintf(concat + strlen(concat), " %d", physicalAddress);
        sprintf(concat + strlen(concat), " %d", lengthRegister);

        int pid = (int)CurrentPCB->pid;
        char* dir = uint32ToChar(physicalAddress);      
        /* char* value = uint32ToChar();                               //NECESITAMOS CONOCER EL VALOR A ESCRIBIR/LEER ANTES DE ENVIARLO A IO O LO PODEMOS CONOCER DESPUES?
        double cantLogs = ceil(lengthRegister/ (float)pagesLength); 
        if( cantLogs <= 1 ){
            logReadMemory(pid, dir, value);
        } else {
            for (i = 0; i < cantLogs; i++){
                physicalAddress += pagesLength;
                dir = uint32ToChar(physicalAddress);
                value = uint32ToChar();                             //NECESITAMOS CONOCER EL VALOR A ESCRIBIR/LEER ANTES DE ENVIARLO A IO O LO PODEMOS CONOCER DESPUES?
                logReadMemory(pid, dir, value);
            }
            
        } */

        //IO_STDOUT_WRITE_FN(concat);
        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, concat);

        // Imprimir la cadena resultante
        //printf("{STDOUT} Este es el resultado final que se envía a kernel: %s\n", concat);

        // Liberar la memoria asignada
        free(buffer);
        free(concat);
        free(secondRegisterName);
        free(interphaceName);
        free(registerName);

    } else if (strcmp(buffer, "IO_FS_CREATE") == 0) {
        interruptPcb = 1;
        // char* iOName = getNextWord(operation, &offset);
        // char* fileName = getNextWord(operation, &offset);

        // Concatenar las palabras leídas en un solo string
        // char* concat = malloc(sizeof(char)* 50); // 3 para los espacios y el terminador nulo
        // if (concat == NULL) {
        //     fprintf(stderr, "Error al asignar memoria\n");
        //     strcpy(PCB->context->reason, "ERROR");
        //     strcpy(PCB->context->info, "ERROR IO_FS_CREATE: Error allocating memory");
        //     return interruptPcb;
        // }
        
        // strcpy(concat, "IO_FS_CREATE ");
        // strcat(concat, iOName);
        // strcat(concat," ");
        // strcat(concat, fileName);

        //sprintf(concat, "IO_FS_CREATE %s %s", iOName, fileName);

        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, operation);
        log_info(log_cpu, "{IO_FS_CREATE} Este es el resultado final: %s\n", operation);
        free(buffer);
        // free(concat);
        // free(iOName);
        // free(fileName);
    } else if (strcmp(buffer, "IO_FS_DELETE") == 0) {
        interruptPcb = 1;
        // char* iOName = getNextWord(operation, &offset);
        // char* fileName = getNextWord(operation, &offset);
        // // Concatenar las palabras leídas en un solo string
        // char* concat = malloc(sizeof(char)* 50); // 3 para los espacios y el terminador nulo
        // if (concat == NULL) {
        //     fprintf(stderr, "Error al asignar memoria\n");
        //     strcpy(PCB->context->reason, "ERROR");
        //     strcpy(PCB->context->info, "ERROR IO_FS_DELETE: Error allocating memory");
        //     return interruptPcb;
        // }
      
        // strcpy(concat, "IO_FS_DELETE ");
        // strcat(concat, iOName);
        // strcat(concat," ");
        // strcat(concat, fileName);

        // sprintf(concat, "IO_FS_DELETE %s %s", iOName, fileName);

        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, operation);
        log_info(log_cpu, "{IO_FS_DELETE} Este es el resultado final: %s\n", operation);
        free(buffer);
        // free(concat);
        // free(iOName);
        // free(fileName);
    } else if (strcmp(buffer, "IO_FS_TRUNCATE") == 0) {
        interruptPcb = 1;
        char* iOName = getNextWord(operation, &offset);
        char* fileName = getNextWord(operation, &offset);
        char* registerName = getNextWord(operation, &offset);
        void* registerValueAddress = getRegister(registerName);

        uint32_t length = 0;
        if (strlen(registerName) == 3 || 
            !strcmp(registerName, "SI") || 
            !strcmp(registerName, "DI") || 
            !strcmp(registerName, "PC") ||
            !strcmp(registerName, "EAX") ||
            !strcmp(registerName, "EBX") ||
            !strcmp(registerName, "ECX") ||
            !strcmp(registerName, "EDX")) {
            length = *(uint32_t*)registerValueAddress;
        } else {
            length = *(uint8_t*)registerValueAddress;
        }

     
        char* concat = malloc(sizeof(char)* 50); 
        if (concat == NULL) {
            fprintf(stderr, "Error al asignar memoria\n");
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR FS_TRUNCATE Error allocating memory");
            return interruptPcb;
        }
 
        sprintf(concat, "IO_FS_TRUNCATE %s %s %u", iOName, fileName, length);
        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, concat);
        log_info(log_cpu, "{IO_FS_TRUNCATE} Este es el resultado final: %s\n", concat);

        free(buffer);
        free(concat);
        free(iOName);
        free(fileName);
    } else if (strcmp(buffer, "IO_FS_WRITE") == 0) {
        interruptPcb = 1;
        //printf("Se reconoce la operación IO_FS_WRITE. \n");
        char* interphaceName = getNextWord(operation, &offset);
        char* fileName = getNextWord(operation, &offset);
        char* registerName = getNextWord(operation, &offset);
        void* registerValueAddress = getRegister(registerName); 
        char* secondRegisterName = getNextWord(operation, &offset); // Leer el nombre del registro
        void* registerValueLength = getRegister(secondRegisterName); // Obtener el puntero devuelto por getRegister
        char* filePointerChar = getNextWord(operation, &offset);
        void* filePointer = getRegister(filePointerChar);

        uint32_t logicalAddress = 0;
        if (registerValueAddress != NULL) {
            if (strlen(registerName) == 3 || 
            !strcmp(registerName, "SI") || 
            !strcmp(registerName, "DI") || 
            !strcmp(registerName, "PC") ||
            !strcmp(registerName, "EAX") ||
            !strcmp(registerName, "EBX") ||
            !strcmp(registerName, "ECX") ||
            !strcmp(registerName, "EDX")) {
                logicalAddress  = *(uint32_t*)registerValueAddress;
            } else {
                logicalAddress = *(uint8_t*)registerValueAddress;
            }
            //printf("Se leyó el registro %s, con valor %u\n", registerName, logicalAddress);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", registerName);
        }
       
        uint32_t lengthRegister = 0; 
        if (registerValueLength != NULL) {
            if (strlen(secondRegisterName) == 3 || 
            !strcmp(secondRegisterName, "SI") || 
            !strcmp(secondRegisterName, "DI") || 
            !strcmp(secondRegisterName, "PC") ||
            !strcmp(secondRegisterName, "EAX") ||
            !strcmp(secondRegisterName, "EBX") ||
            !strcmp(secondRegisterName, "ECX") ||
            !strcmp(secondRegisterName, "EDX")) {
                lengthRegister  = *(uint32_t*)registerValueLength;
            } else {
                lengthRegister = *(uint8_t*)registerValueLength;
            }
            //printf("Se leyó el registro %s, con longitud %u\n", secondRegisterName, lengthRegister);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", secondRegisterName);
        }

        //printf("El tamaño a leer de FS_WRITE es:%d\n", lengthRegister);
       

        uint32_t physicalAddress = logicalToPhysicalAddress(logicalAddress, CurrentPCB->pid);

        uint32_t trueFilePointer = 0;

        if (filePointer != NULL) {
            if (strlen(filePointer) == 3 || 
            !strcmp(filePointerChar, "SI") || 
            !strcmp(filePointerChar, "DI") || 
            !strcmp(filePointerChar, "PC") ||
            !strcmp(filePointerChar, "EAX") ||
            !strcmp(filePointerChar, "EBX") ||
            !strcmp(filePointerChar, "ECX") ||
            !strcmp(filePointerChar, "EDX")) {
                trueFilePointer  = filePointer;
            } else {
                trueFilePointer = *(uint8_t*)filePointer;
            }
            //printf("filePointerChar: %s\n", filePointerChar);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", filePointerChar);
        }

        // Asignar memoria para el buffer
        char* concat = malloc(sizeof(char)*100);
        if (concat == NULL) {
            log_error(log_cpu, "Unable to allocate memory");
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR IO_FS_WRITE: Error allocating memory");
            return 1;
        }

        // Formatear la cadena en el buffer
        // strcpy(concat, "IO_FS_WRITE ");
        // strcat(concat, interphaceName);
        // strcat(concat, " ");
        // strcat(concat, fileName);
        // sprintf(concat + strlen(concat), " %d", physicalAddress);
        // sprintf(concat + strlen(concat), " %d", lengthRegister);
        // sprintf(concat + strlen(concat), " %d", trueFilePointer);


        sprintf(concat, "IO_FS_WRITE %s %s %u %u %u", interphaceName, fileName, physicalAddress, lengthRegister, trueFilePointer);

        //IO_STDOUT_WRITE_FN(concat);
        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, concat);

        // Imprimir la cadena resultante
        log_info(log_cpu, "{FS_WRITE} Este es el resultado final que se envía a kernel: %s\n", concat);

        // Liberar la memoria asignada
        free(buffer);
        free(concat);
        free(secondRegisterName);
        free(interphaceName);
        free(registerName);
        free(filePointerChar);
    } else if (strcmp(buffer, "IO_FS_READ") == 0) {
        interruptPcb = 1;
        //printf("Se reconoce la operación IO_FS_READ \n");
        char* interphaceName = getNextWord(operation, &offset);
        char* fileName = getNextWord(operation, &offset);
        char* registerName = getNextWord(operation, &offset);
        void* registerValueAddress = getRegister(registerName); 
        char* secondRegisterName = getNextWord(operation, &offset); // Leer el nombre del registro
        void* registerValueLength = getRegister(secondRegisterName); // Obtener el puntero devuelto por getRegister
        char* filePointerChar = getNextWord(operation, &offset);
        void* filePointer = getRegister(filePointerChar);

        uint32_t logicalAddress = 0;
        if (registerValueAddress != NULL) {
            if (strlen(registerName) == 3 || 
            !strcmp(registerName, "SI") || 
            !strcmp(registerName, "DI") || 
            !strcmp(registerName, "PC") ||
            !strcmp(registerName, "EAX") ||
            !strcmp(registerName, "EBX") ||
            !strcmp(registerName, "ECX") ||
            !strcmp(registerName, "EDX")) {
            logicalAddress  = *(uint32_t*)registerValueAddress;
            } else {
                logicalAddress = *(uint8_t*)registerValueAddress;
            }
            //printf("Se leyó el registro %s, con valor %u\n", registerName, logicalAddress);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", registerName);
        }
       
        uint32_t lengthRegister = 0; 
        if (registerValueLength != NULL) {
            if (strlen(secondRegisterName) == 3 || 
            !strcmp(secondRegisterName, "SI") || 
            !strcmp(secondRegisterName, "DI") || 
            !strcmp(secondRegisterName, "PC") ||
            !strcmp(secondRegisterName, "EAX") ||
            !strcmp(secondRegisterName, "EBX") ||
            !strcmp(secondRegisterName, "ECX") ||
            !strcmp(secondRegisterName, "EDX")) {
                lengthRegister  = *(uint32_t*)registerValueLength;
            } else {
                lengthRegister = *(uint8_t*)registerValueLength;
            }
            //printf("Se leyó el registro %s, con longitud %u\n", secondRegisterName, lengthRegister);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", secondRegisterName);
        }

        //printf("El tamaño a leer de STDOUT es:%d\n", lengthRegister);
       

        uint32_t physicalAddress = logicalToPhysicalAddress(logicalAddress, CurrentPCB->pid);

        uint32_t trueFilePointer = 0;

        if (filePointer != NULL) {
            if (strlen(filePointer) == 3 || 
            !strcmp(filePointerChar, "SI") || 
            !strcmp(filePointerChar, "DI") || 
            !strcmp(filePointerChar, "PC") ||
            !strcmp(filePointerChar, "EAX") ||
            !strcmp(filePointerChar, "EBX") ||
            !strcmp(filePointerChar, "ECX") ||
            !strcmp(filePointerChar, "EDX")) {
                trueFilePointer  = filePointer;
            } else {
                trueFilePointer = *(uint8_t*)filePointer;
            }
            //printf("filePointerChar: %s\n", filePointerChar);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", filePointerChar);
        }

        // Asignar memoria para el buffer
        char* concat = malloc(sizeof(char)*100);
        if (concat == NULL) {
            log_error(log_cpu, "Unable to allocate memory");
            strcpy(PCB->context->reason, "ERROR");
            strcpy(PCB->context->info, "ERROR IO_STDOUT_WRITE: Error allocating memory");
            return 1;
        }

        // Formatear la cadena en el buffer
        // strcpy(concat, "IO_FS_READ ");
        // strcat(concat, interphaceName);
        // strcat(concat, " ");
        // strcat(concat, fileName);
        // sprintf(concat + strlen(concat), " %d", physicalAddress);
        // sprintf(concat + strlen(concat), " %d", lengthRegister);
        // sprintf(concat + strlen(concat), " %d", trueFilePointer);

        sprintf(concat, "IO_FS_READ %s %s %u %u %u", interphaceName, fileName, physicalAddress, lengthRegister, trueFilePointer);

        //IO_STDOUT_WRITE_FN(concat);
        strcpy(PCB->context->reason, "IO");
        strcpy(PCB->context->info, concat);

        // Imprimir la cadena resultante
        log_info(log_cpu, "{STDOUT} Este es el resultado final que se envía a kernel: %s\n", concat);

        // Liberar la memoria asignada
        free(buffer);
        free(concat);
        free(secondRegisterName);
        free(interphaceName);
        free(registerName);
        free(filePointerChar);
    }
    
    else if (strcmp(buffer, "EXIT") == 0) {
        interruptPcb = 1;
        strcpy(PCB->context->reason, "EXIT");
        strcpy(PCB->context->info, "");
        free(buffer);  // VER SI SOLUCIONA MEMORY LEAK 2250 BYTES
    } 
    else {
        free(buffer);
    }
    return interruptPcb;
}

// char* getNextWord(char* operation, int* offset){
//     char* buffer = malloc(sizeof(char)*6);
//     int i = 0;
//     while (operation[(*offset)] != '\0' && operation[(*offset)] != ' ' && operation[(*offset)] != '\n'){            // Lee el la palabra hasta el espacio o fin de cadena
//         char caracter = operation[(*offset)];                                                                       // Copia caractér por caracter lo leido
//         buffer[i] = operation[(*offset)];                                                                           // Pega caractér copiado en el buffer
//         (*offset)++;                                                                                                // Mueve la posicion de la siguiente lectura
//         i++;
//     }
//     buffer[i] = '\0';
//     (*offset)++;                                                                    // Deja fuera al espacio o fin de cadena 
//     return buffer;
// }

uint32_t charToUint32(char* str) {
    uint32_t regValue;
    char* binary32bits = malloc(33);
    if(strlen(str) > 4) {
        log_error(logger, "Como se devuelve un valor de 32 bits, solo se pueden convertir hasta 4 caracteres");
        return;
    }
    for (int i = 0; i < 4; i++) {
        char caracter = str[i];
        int ascii = (int)caracter;
        char* binary = intToBinary8Bits(ascii);
        sprintf(binary32bits, "%s%s", binary32bits, binary);
        free(binary);
    }
    regValue = (uint32_t)binaryToInt32Bits(binary32bits);
    free(binary32bits);
    return regValue;
}

char* uint32ToChar(uint32_t regValue) {
    char* str = malloc(5);
    char* binary32bits = intToBinary32Bits((int)regValue);
    for (int i = 0; i < 4; i++) {
        char* binary8bits = (char*)malloc(8 + 1);
        char caracter;
        strncpy(binary8bits, binary32bits, 8);
        binary8bits[8] = '\0';
        memmove(binary32bits, binary32bits + 8, strlen(binary32bits) - 8 + 1);
        caracter = (char)binaryToInt8Bits(binary8bits);
        str[i] = caracter;
        free(binary8bits);
    }
    str[4] = '\0';
    return str;
}

void* getRegister(char* buffer){                            // Recibe char* con el nombre del registro y lo evalua para devolver un puntero al registro

    if(strcmp(buffer, "AX") == 0) {
        return CurrentPCB->cpuRegisters.AX;
    } else if(strcmp(buffer, "BX") == 0) {
        return CurrentPCB->cpuRegisters.BX;
    } else if(strcmp(buffer, "CX") == 0) {
        return CurrentPCB->cpuRegisters.CX;
    } else if(strcmp(buffer, "DX") == 0) {
        return CurrentPCB->cpuRegisters.DX;
    } else if(strcmp(buffer, "EAX") == 0) {
        return CurrentPCB->cpuRegisters.EAX;
    } else if(strcmp(buffer, "EBX") == 0) {
        return CurrentPCB->cpuRegisters.EBX;
    } else if(strcmp(buffer, "ECX") == 0) {
        return CurrentPCB->cpuRegisters.ECX;
    } else if(strcmp(buffer, "EDX") == 0) {
        return CurrentPCB->cpuRegisters.EDX;
    } else if(strcmp(buffer, "SI") == 0) {
        return CurrentPCB->cpuRegisters.SI;
    } else if(strcmp(buffer, "DI") == 0) {
        return CurrentPCB->cpuRegisters.DI;
    } else if(strcmp(buffer, "PC") == 0) {
        return CurrentPCB->cpuRegisters.PC;
    } else {
       // Comportamiento ante el caso erroneo en el que buffer no contenga el nombre registro
    }

}


void SET(char* registerName, int value){

    if (strlen(registerName) == 3 || !strcmp(registerName, "SI") || !strcmp(registerName, "DI") || !strcmp(registerName, "PC")){  // Match con PC, registros E.. , SI y DI
		uint32_t *destiny = getRegister(registerName);
        //printf("Se reconoce un registro de 32 bits de nombre %s. \n", registerName);
		*destiny = value;
	} else {                                               // Match con AX, BX, CX y DX
		uint8_t *destiny = getRegister(registerName);
        //printf("Se reconoce un registro de 8 bits de nombre %s. \n", registerName);
        *destiny = value;
	}
    
}

void MOV_IN(char* registerData, char* registerAdress){

        
        void* srcAddress = getRegister(registerAdress);
        //printf("Se recibio el registro %s. \n", registerAdress);

        uint32_t DL = 0;

        if (srcAddress != NULL) {
            if (strlen(registerAdress) == 3 || 
            !strcmp(registerAdress, "SI") || 
            !strcmp(registerAdress, "DI") || 
            !strcmp(registerAdress, "PC")) {
                DL  = *(uint32_t*)srcAddress;
            } else {
                DL = *(uint8_t*)srcAddress;
            }
            //printf("Se leyó el registro %s, con valor %d\n", registerAdress, DL);
        } else {
            log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", registerAdress);
        }

        

        uint32_t destiny;

        if (strlen(registerData) == 3 || !strcmp(registerData, "SI") || !strcmp(registerData, "DI") ||  !strcmp(registerData, "PC")) {
            destiny = 4;
            ejecutar_mov_in_cuatro_bytes(registerData, DL, CurrentPCB->pid);
        } else {
            //Leer 1 bytes (8bits)
            uint32_t phySrc = logicalToPhysicalAddress(DL, CurrentPCB->pid);
            destiny = 1;
            MOVE_IN_TO_MEMORY_params* newSendMoveIn = NEW_MOVE_IN_TO_MEMORY(CurrentPCB->pid, destiny,  phySrc);
            send_MOVE_IN_TO_MEMORY(newSendMoveIn, memoryConnection);

            free(newSendMoveIn);  // VER SI SOLUCIONA MEMORY LEAK 72 BYTES

            uint32_t datoAEscribir;
            recvInt(&datoAEscribir, memoryConnection);
            SET(registerData, datoAEscribir);
        }
    
}

void ejecutar_mov_in_cuatro_bytes(char* reg_datos, uint32_t dir_logica_p, uint32_t pid){

    uint32_t dir_logica = dir_logica_p;
    int nPage = floor(dir_logica / pagesLength); 
	int desplazamiento = dir_logica  - nPage * pagesLength;

    int cant_paginas_a_traer = (desplazamiento + 4 > pagesLength)? 2 : 1;

    if (cant_paginas_a_traer == 1) {
        uint32_t dir_fisica = UINT32_MAX;
        dir_fisica = logicalToPhysicalAddress(dir_logica, pid);
        uint32_t valor_leido = 0;
        MOVE_IN_TO_MEMORY_params* newSendMoveIn = NEW_MOVE_IN_TO_MEMORY(pid, 4,  dir_fisica);
        send_MOVE_IN_TO_MEMORY(newSendMoveIn, memoryConnection);
        uint32_t datoALEER;
        recvInt(&datoALEER, memoryConnection);
        SET(reg_datos, datoALEER);
    } else {
        // caso 2 paginas leidas
        uint32_t valor_leido = leer_y_guardar_de_dos_paginas(dir_logica, pid);
        SET(reg_datos, valor_leido);
    }
 }

uint32_t leer_y_guardar_de_dos_paginas(uint32_t dir_logica_P, uint32_t pid){
    
    uint32_t dir_logica = dir_logica_P;
    int nPage = floor(dir_logica / pagesLength); 
	int desplazamiento = dir_logica  - nPage * pagesLength;
    uint32_t dir_fisica_primera = UINT32_MAX;
    dir_fisica_primera = logicalToPhysicalAddress(dir_logica, pid);
    int cant_bytes_a_leer_primera_pag = 4 - (desplazamiento + 4 - pagesLength);
//                                           |---------------- saco por cuanto se paso ---------------------|
//                                     |----- la diferencia con el tamaño del registro me da cuanto tengo que leer

    // PRIMER PAGINA

    uint32_t primera_lectura = 0;
    MOVE_IN_TO_MEMORY_params* newSendMoveIn_1 = NEW_MOVE_IN_TO_MEMORY(pid, cant_bytes_a_leer_primera_pag,  dir_fisica_primera);
    send_MOVE_IN_TO_MEMORY(newSendMoveIn_1, memoryConnection);
    recvInt(&primera_lectura, memoryConnection);

    // SEGUNDA PAGINA

    uint32_t dir_logica_segunda = dir_logica + cant_bytes_a_leer_primera_pag; // asi completo los bytes de la primera y me situo en la segunda desde el comienzo
    int32_t dir_fisica_segunda = UINT32_MAX;
    dir_fisica_segunda = logicalToPhysicalAddress(dir_logica_segunda, pid);

    int cant_bytes_a_leer_segunda_pag = 4 - cant_bytes_a_leer_primera_pag;

    uint32_t segunda_lectura = 0;
    MOVE_IN_TO_MEMORY_params* newSendMoveIn_2 = NEW_MOVE_IN_TO_MEMORY(pid, cant_bytes_a_leer_segunda_pag ,  dir_fisica_segunda);
    send_MOVE_IN_TO_MEMORY(newSendMoveIn_2, memoryConnection);
    recvInt(&segunda_lectura, memoryConnection);

    uint32_t valor_reconstruido = dato_reconstruido(primera_lectura, segunda_lectura, cant_bytes_a_leer_primera_pag, cant_bytes_a_leer_segunda_pag);

    return valor_reconstruido;
}

uint32_t dato_reconstruido(uint32_t primera, uint32_t segunda, int bytes_primera, int bytes_segunda){

    void* primera_ptr = &primera;
    void* segunda_ptr = &segunda;
    void* dato_reconstruido_ptr = malloc(4);
    memset(dato_reconstruido_ptr, 0, 4);

    //printf("\nDato antes de ser reconstruido:\n");
	//mem_hexdump(dato_reconstruido_ptr, 4);

    memcpy(dato_reconstruido_ptr, primera_ptr, (size_t)bytes_primera);
    memcpy(dato_reconstruido_ptr + (size_t)bytes_primera, segunda_ptr, (size_t)bytes_segunda);

    //printf("\nDato despues de ser reconstruido:\n");
	//mem_hexdump(dato_reconstruido_ptr, 4);

    uint32_t valor = *(uint32_t*)dato_reconstruido_ptr;

    free(dato_reconstruido_ptr);
    return valor;
}





void MOV_OUT(char* registerAdress, char* registerData) {
    void* srcAddress = getRegister(registerAdress);
    uint32_t DL = 0;

    if (srcAddress != NULL) {
        if (strlen(registerAdress) == 3 || 
            !strcmp(registerAdress, "SI") || 
            !strcmp(registerAdress, "DI") || 
            !strcmp(registerAdress, "PC")) {
            DL = *(uint32_t*)srcAddress;
        } else {
            DL = *(uint8_t*)srcAddress;
        }
    } else {
        log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", registerAdress);
        return;
    }


    void* srcData = getRegister(registerData);
    uint32_t srcDataInt = 0;
    int tamanioParaLeer = 0;

    if (srcData != NULL) {
        if (strlen(registerData) == 3 || 
            !strcmp(registerData, "SI") || 
            !strcmp(registerData, "DI") || 
            !strcmp(registerData, "PC")) {
            srcDataInt = *(uint32_t*)srcData;
            tamanioParaLeer = 4;
        } else {
            srcDataInt = *(uint8_t*)srcData;
            tamanioParaLeer = 1;
        }

        if (tamanioParaLeer == 4 && pagina_dividida(DL) == 1) {
            print_rectangle("SE VA A ESCRIBIR EN 2 PAGINAS!");
            escribir_y_guardar_en_dos_paginas(DL, srcDataInt, CurrentPCB->pid);
        } else {
            uint32_t phySrc = logicalToPhysicalAddress(DL, CurrentPCB->pid);
            MOVE_OUT_TO_MEMORY_params* newSendMoveOUT = NEW_MOVE_OUT_TO_MEMORY(CurrentPCB->pid, tamanioParaLeer, phySrc, srcDataInt);
            send_MOVE_OUT_TO_MEMORY(newSendMoveOUT, memoryConnection);
            free(newSendMoveOUT);  // VER SI SOLUCIONA MEMORY LEAK 480 BYTES
        }
    } else {
        log_error(log_cpu, "Error: No se pudo obtener el registro %s\n", registerData);
    }
}

void escribir_y_guardar_en_dos_paginas(uint32_t dir_logica_destino_param, uint32_t valor, uint32_t pid) {
    int nPage = floor(dir_logica_destino_param / pagesLength); 
    int desplazamiento = dir_logica_destino_param - nPage * pagesLength;

    int bytes_a_escribir_primera_pag = pagesLength - desplazamiento;
    if (bytes_a_escribir_primera_pag > 4) {
        bytes_a_escribir_primera_pag = 4;
    }
    int bytes_a_escribir_segunda_pag = 4 - bytes_a_escribir_primera_pag;

    uint16_t valor_parte_1 = (uint16_t)(valor & 0xFFFF);
    uint16_t valor_parte_2 = (uint16_t)((valor >> 16) & 0xFFFF);

    uint32_t dir_fisica_destino = logicalToPhysicalAddress(dir_logica_destino_param, pid);
    MOVE_OUT_TO_MEMORY_params* newSendMoveOUT = NEW_MOVE_OUT_TO_MEMORY(pid, bytes_a_escribir_primera_pag, dir_fisica_destino, valor_parte_1);
    send_MOVE_OUT_TO_MEMORY(newSendMoveOUT, memoryConnection);

    uint32_t dir_logica_segunda_pag = dir_logica_destino_param + bytes_a_escribir_primera_pag;
    uint32_t dir_fisica_destino_segunda_pag = logicalToPhysicalAddress(dir_logica_segunda_pag, pid);
    newSendMoveOUT = NEW_MOVE_OUT_TO_MEMORY(pid, bytes_a_escribir_segunda_pag, dir_fisica_destino_segunda_pag, valor_parte_2);
    send_MOVE_OUT_TO_MEMORY(newSendMoveOUT, memoryConnection);
}

int pagina_dividida(uint32_t dir_logica) {
    int nPage = floor(dir_logica / pagesLength); 
    int desplazamiento = dir_logica - nPage * pagesLength;
    if (desplazamiento + 4 > pagesLength) {
        return 1;
    }
    return 0;
}

void SUM(char*  destinyRegister, char*  originRegister){

    if (strlen(destinyRegister) == 3 || !strcmp(destinyRegister, "SI") || !strcmp(destinyRegister, "DI") || !strcmp(destinyRegister, "PC")){ 
		uint32_t *destiny = getRegister(destinyRegister);
        //printf("Se reconoce un registro de 32 bits de nombre %s. \n", destinyRegister);

        if (strlen(originRegister) == 3 || !strcmp(originRegister, "SI") || !strcmp(originRegister, "DI") || !strcmp(originRegister, "PC")){  
		    uint32_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 32 bits de nombre %s. \n", originRegister);
            *destiny = *destiny + *origin;
	    } else {       
		    uint8_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 8 bits de nombre %s. \n", originRegister);
            *destiny = *destiny + *origin;
        }
	} else {      
		uint8_t *destiny = getRegister(destinyRegister);
        //printf("Se reconoce un registro de 8 bits de nombre %s. \n", destinyRegister);

        if (strlen(originRegister) == 3 || !strcmp(originRegister, "SI") || !strcmp(originRegister, "DI") || !strcmp(originRegister, "PC")){  
		    uint32_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 32 bits de nombre %s. \n", originRegister);
            if(*origin <= 255){
                //printf("Se castea el registro de tamaño 32 a tamaño 8. \n");
                *destiny = *destiny + (uint8_t) *origin;
            } else {
                //MANEJO DE ERROR EN CASO DE OVERFLOW
            }
	    } else {       
		    uint8_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 8 bits de nombre %s. \n", originRegister);
            *destiny = *destiny + *origin;
        }
	}

}

void SUB(char* destinyRegister, char* originRegister){

    if (strlen(destinyRegister) == 3 || !strcmp(destinyRegister, "SI") || !strcmp(destinyRegister, "DI") || !strcmp(destinyRegister, "PC")){ 
		uint32_t *destiny = getRegister(destinyRegister);
        //printf("Se reconoce un registro de 32 bits de nombre %s. \n", destinyRegister);

        if (strlen(originRegister) == 3 || !strcmp(originRegister, "SI") || !strcmp(originRegister, "DI") || !strcmp(originRegister, "PC")){  
		    uint32_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 32 bits de nombre %s. \n", originRegister);
            *destiny = *destiny - *origin;
	    } else {       
		    uint8_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 8 bits de nombre %s. \n", originRegister);
            *destiny = *destiny - *origin;
        }
	} else {      
		uint8_t *destiny = getRegister(destinyRegister);
        //printf("Se reconoce un registro de 8 bits de nombre %s. \n", destinyRegister);

        if (strlen(originRegister) == 3 || !strcmp(originRegister, "SI") || !strcmp(originRegister, "DI") || !strcmp(originRegister, "PC")){  
		    uint32_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 32 bits de nombre %s. \n", originRegister);
            if(*origin <= 255){
                //printf("Se castea el registro de tamaño 32 a tamaño 8. \n");
                *destiny = *destiny + (uint8_t) *origin;
            } else {
                //MANEJO DE ERROR
            }
	    } else {       
		    uint8_t *origin = getRegister(originRegister);
            //printf("Se reconoce un registro de 8 bits de nombre %s. \n", originRegister);
            *destiny = *destiny - *origin;
        }
	}

}

void JNZ(char* registerName, int instruction){

    if (strlen(registerName) == 3 || !strcmp(registerName, "SI") || !strcmp(registerName, "DI") || !strcmp(registerName, "PC")){  
		uint32_t *origin = getRegister(registerName);
        //printf("Se reconoce un registro de 32 bits de nombre %s. \n", registerName);
        if (*origin != 0){
            *(CurrentPCB->cpuRegisters.PC) = instruction; 
        }  
	} else {       
		uint8_t *origin = getRegister(registerName);
        //printf("Se reconoce un registro de 8 bits de nombre %s. \n", registerName);
        if (*origin != 0){
            *(CurrentPCB->cpuRegisters.PC) = instruction; 
        } 
    }
    
}

void COPY_STRING_(int size){

    if (size <= 0) {

        log_error(log_cpu, "Tamaño inválido para copiar.\n");
    } else {

        uint32_t phySrc = logicalToPhysicalAddress(*(CurrentPCB->cpuRegisters.SI), CurrentPCB->pid);
        uint32_t phyDest = logicalToPhysicalAddress(*(CurrentPCB->cpuRegisters.DI), CurrentPCB->pid);

        char* src = (char*)phySrc;
        char* dest = (char*)phyDest;

        //Necesito leer una determinada cantidad de bytes desde el comienzo de la direccion de src

        memcpy(dest, src, size);
    }

}

void RE_SIZE(int size){
    
    int code_op = RESIZE;
        
    //printf("Pido tamaño de paginas\n");
   
    //envio codigo de operacion RESIZE a memoria para avisarle que me pase el tamaño de paginas
    send(memoryConnection, &code_op, sizeof(int32_t), 0);
      
    //aca espero el tamaño de paginas, pagesLength inicia en -1 (en main.h) contemplado de a entrada caso de error
    recv(memoryConnection, &pagesLength, sizeof(int32_t) , MSG_WAITALL);

    if(pagesLength != -1){
        //printf("Tamaño de paginas recibido: '%d'\n", pagesLength);
        // send(memoryConnection ,1 ,sizeof(int32_t), 0); //Solo le aviso a memoria que recibí bien el tamaño de pagina
    } else {
        log_error(log_cpu, "Error al recibir tamaño de pagina");
    }

}

void IO_GEN_SLEEP(char* iOInstruction){   

    sendFunctionIo(iOInstruction, kernelSocket);

}

void IO_STDOUT_WRITE_FN(char* iOInstruction)
{   

    sendFunctionIo(iOInstruction, kernelSocket);
    

}

void IO_STDIN_READ_FN(char* iOInstruction)
{   

    sendFunctionIo(iOInstruction, kernelSocket);
    

}


