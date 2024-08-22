#include "utils_io.h"

void inicializarConfig(char* configArchive) {
    ioConfig = initConfig(configArchive);
    IP_KERNEL = getConfigValue(ioConfig, "IP_KERNEL");
    PUERTO_KERNEL = getConfigValue(ioConfig, "PUERTO_KERNEL");
    IP_MEMORIA = getConfigValue(ioConfig, "IP_MEMORIA");
    PUERTO_MEMORIA = getConfigValue(ioConfig, "PUERTO_MEMORIA");
    TIPO_INTERFAZ = getConfigValue(ioConfig, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = getConfigValue(ioConfig, "TIEMPO_UNIDAD_TRABAJO");
    PATH_BASE_DIALFS = getConfigValue(ioConfig, "PATH_BASE_DIALFS");
    BLOCK_SIZE = getConfigValue(ioConfig, "BLOCK_SIZE");
    BLOCK_COUNT = getConfigValue(ioConfig, "BLOCK_COUNT");
    RETRASO_COMPACTACION = getConfigValue(ioConfig, "RETRASO_COMPACTACION");

    log_info(logger, "IP_KERNEL: %s", IP_KERNEL);
    log_info(logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    log_info(logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);
    log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %s", TIEMPO_UNIDAD_TRABAJO);
    log_info(logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    log_info(logger, "BLOCK_SIZE: %s", BLOCK_SIZE);
    log_info(logger, "BLOCK_COUNT: %s", BLOCK_COUNT);
    log_info(logger, "RETRASO_COMPACTACION: %s", RETRASO_COMPACTACION);
}

int createInterface() {
    char tempName[256]; // Usamos un buffer temporal para leer la entrada del usuario
    int choice;
    int valid_choice = 0; // Variable para controlar la validez de la elección

    printf("Ingresa el nombre de la interfaz: ");
    scanf("%s", tempName);

    // Asigna memoria dinámica para el nombre de la interfaz y copia el valor leído
    interfaceName = strdup(tempName);
    if (interfaceName == NULL) {
        perror("Error al asignar memoria para el nombre de la interfaz");
        exit(1);
    }
    int isDialFS = 0;
    while (!valid_choice) {

        printf("Elige una interfaz (path):\n");
        printf("1 - generic.config\n");
        printf("2 - stdin.config\n");
        printf("3 - stdout.config\n");
        printf("4 - dialfs.config\n");
        printf("5 - SP1.config\n");
        printf("6 - ESPERA.config\n");
        printf("Ingresa 0 para salir\n");
        printf("Opción: ");
        scanf("%d", &choice);
        

        switch (choice) {
            case 1:
                pathConfig = "../IOGEN.config";
                valid_choice = 1;
                isDialFS = 0;
                break;
            case 2:
                pathConfig = "../STDIN.config";
                valid_choice = 1;
                isDialFS = 0;
                break;
            case 3:
                pathConfig = "../STDOUT.config";
                valid_choice = 1;
                isDialFS = 0;
                break;
            case 4:
                pathConfig = "../DIALFS.config";
                valid_choice = 1;
                isDialFS = 1;
                break;
            case 5:
                pathConfig = "../SP1.config";
                valid_choice = 1;
                isDialFS = 0;
                break;
            case 6:
                pathConfig = "../ESPERA.config";
                valid_choice = 1;
                isDialFS = 0;
                break;
            case 0:
                printf("Saliendo del programa.\n");
                free(interfaceName); // Liberar la memoria asignada antes de salir
                exit(0);
            default:
                printf("No es un valor válido. Por favor, ingresa los valores 1, 2, 3, 4 o 0 para salir.\n");
        }
    }

    if (access(pathConfig, F_OK) != -1) {
        inicializarConfig(pathConfig);
        log_info(logger, "Interfaz '%s' creada con la configuración '%s'.\n", interfaceName, pathConfig);
        if (isDialFS) {
            fileBlocksSize = 0;
            fileBitmapSize = 0;
            
            path_blocks = malloc(sizeof(char)*100);
            path_bitmap = malloc(sizeof(char)*100);
            path_metadata = malloc(sizeof(char)*100);

            char* basePath = PATH_BASE_DIALFS;
            basePath = !endsWithSlash(PATH_BASE_DIALFS) ? strcat(basePath, "/") : basePath;
            sprintf(path_blocks, "%sbloques.dat", basePath);
            sprintf(path_bitmap, "%sbitmap.dat", basePath);
            sprintf(path_metadata, "%smetadata", basePath);

            if(ensure_directory(PATH_BASE_DIALFS)) {
                if (!ensure_directory(path_metadata)) {
                    log_error(logger, "Error al crear carpeta path: %s\n", path_metadata);
                    exit(EXIT_FAILURE);
                }
            } else {
                log_error(logger, "Error al crear carpeta path: %s\n", PATH_BASE_DIALFS);
                exit(EXIT_FAILURE);
            }

            createFileBlocks();
            initializeFileBitarray();

            // char* fileName1 = malloc(sizeof(char)*20);
            // char* fileName2 = malloc(sizeof(char)*20);
            // char* fileName3 = malloc(sizeof(char)*20);
            // strcpy(fileName1, "archivoNuevo1");
            // strcpy(fileName2, "archivoNuevo2");
            // strcpy(fileName3, "archivoNuevo3");

            // fsCreate(fileName1, 1);
            // fsTruncate(fileName1, 16852);
            // char* data = malloc(sizeof(char)*20);
            // strcpy(data, "Hola DaniI");
            // fsWrite(fileName1, 0, data);
            // char* readed = fsRead(fileName1, 0, 9);
            // printf("Dato leido: %s\n", readed);
            // //fsTruncate(fileName1, 65);
            // free(readed);
            
            // //char* dataReaded = fsRead(fileName1, 2, 10);
            // //printf("readed: %s\n", dataReaded);
            // // closeFCB(fileName1, 1);
            // // fsDelete(fileName1);
            
            // strcpy(data, "Hola ErikSDSAdfasasf");
            // fsCreate(fileName2, 2);
            // fsTruncate(fileName2, 893);
            // fsWrite(fileName2, 0, data);
            // readed = fsRead(fileName2, 0, 9);
            // printf("El dato leido es %s\n", readed);

            // fsTruncate(fileName1, 17000);

            // // uint32_t newInitBlock = canRelocate(fileName1, 265);
            // // realocate(fileName1, newInitBlock, 16959);
            
            // fsCreate(fileName3, 3);

            // fsTruncate(fileName3, 500);
            // fsTruncate(fileName1, 200);

            // realocate(fileName2, 500, 893);
            // compaction();

            // free(fileName1);
            // free(fileName2);
            // free(fileName3);
        }
        return 0;
    } else {
        // El path no existe
        log_error(logger, "El path '%s' no existe.\n", pathConfig);
        return -1;
    }
}

fcb* createFcb(char* fileName, uint32_t pid) {
    fcb* newFcb = malloc(sizeof(fcb));
    if (newFcb  == NULL) {
        return NULL;
    }

    newFcb->fileName = malloc(sizeof(char)*50);
    strcpy(newFcb->fileName, fileName);
    newFcb->pid = pid;
    return newFcb;
}

void deleteFcb(fcb* fcbToDelete) {
    free(fcbToDelete->fileName);
    free(fcbToDelete);
}

void createFileBlocks() {    
    //printf("Creando bloques.dat\n");
    int fd = open(path_blocks, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        log_error(logger, "Error al abrir/crear el archivo bloques.dat");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        log_error(logger, "Error al obtener información del archivo bloques.dat");
        close(fd);
        return;
    }

    size_t requiredSize = getValue(BLOCK_COUNT) * getValue(BLOCK_SIZE);

    if (st.st_size < requiredSize) {
        // Si el archivo es más pequeño de lo necesario, ajusta su tamaño
        if (ftruncate(fd, requiredSize) == -1) {
            log_error(logger, "Error al ajustar el tamaño del archivo bloques.dat");
            close(fd);
            return;
        }
    }

    char* blocksPointer = mmap(NULL, requiredSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (blocksPointer == MAP_FAILED) {
        log_error(logger, "Error al mapear el archivo bloques.dat en memoria");
        close(fd);
        return;
    }

    if (st.st_size < requiredSize) {
        // Si el archivo era más pequeño y fue extendido, inicializa el nuevo espacio
        memset(blocksPointer, 0, requiredSize);
    }

    blocks = bitarray_create_with_mode(blocksPointer, fileBlocksSize, MSB_FIRST);
    if (!blocks) {
        log_error(logger, "Error al asignar memoria para el objeto bitmap\n");
        munmap(blocksPointer, fileBlocksSize);
        close(fd);
        return;
    }
    
    close(fd);
}

void initializeFileBitarray() {
    log_info(logger, "Creando bitmap.dat\n");

    int fd_bitarray = open(path_bitmap, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitarray == -1) {
        log_error(logger, "No se pudo abrir/crear el archivo bitmap.dat\n");
        return;
    }

    struct stat st;
    if (fstat(fd_bitarray, &st) == -1) {
        log_error(logger, "Error al obtener información del archivo bitmap.dat\n");
        close(fd_bitarray);
        return;
    }

    size_t blockCountInBytes = getValue(BLOCK_COUNT) / 8;

    if (st.st_size < blockCountInBytes) {
        // Si el archivo es más pequeño de lo necesario, ajusta su tamaño
        if (ftruncate(fd_bitarray, blockCountInBytes) == -1) {
            log_error(logger, "Error al ajustar el tamaño del archivo bitmap.dat\n");
            close(fd_bitarray);
            return;
        }
    }

    char* bitmapPointer = mmap(NULL, blockCountInBytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitarray, 0);
    if (bitmapPointer == MAP_FAILED) {
        log_error(logger, "Error al mapear el archivo bitmap.dat en memoria\n");
        close(fd_bitarray);
        return;
    }

    if (st.st_size < blockCountInBytes) {
        // Si el archivo era más pequeño y fue extendido, inicializa el nuevo espacio
        memset(bitmapPointer, 0, blockCountInBytes);
    }

    bitmap = bitarray_create_with_mode(bitmapPointer, blockCountInBytes, MSB_FIRST);
    if (!bitmap) {
        log_error(logger, "Error al asignar memoria para el objeto bitmap\n");
        munmap(bitmapPointer, blockCountInBytes);
        close(fd_bitarray);
        return;
    }

    close(fd_bitarray);
}



void fsCreate (char* fileName, uint32_t pid) {
    if (metadataExist(fileName)) {
        log_warning(logger, "Ya existe archivo %s, no hace falta crear\n", fileName);
        fcb* newFile = createFcb(fileName, pid);
        list_add(globalOpenFiles, newFile);
        return;
    }
    //printf("Creando FS %s\n", fileName);

    // int blockIndex = getFreeBlockIndex();
    // if (blockIndex == -1) {
    //     log_error(logger, "No hay bloque libre para asignar al archivo %s\n", fileName);
    //     free(fileName);
    //     return;
    // }
    
    createMetadata(fileName); // En realida no hace falta guardar metadata, ya que despues es posible acceder por el fileName del FCB, en teoria tienen el mismo nombre
    //t_file* newFile = createFile(fileName, metadata);
    fcb* newFile = createFcb(fileName, pid); // Por ahora el tamaño del archivo esta vacio

    if (newFile == NULL) {
        log_error(logger, "No se pudo crear nuevo FCB\n");
    }

    log_info(logger, "DialFS - Crear Archivo: PID: %d - Crear Archivo: %s", pid, fileName);

    //assignBit(bitmap, blockIndex);
    list_add(globalOpenFiles, newFile);

    //free(fileName);
}

int fsTruncate(char* fileName, uint32_t newSize) {
    if (!metadataExist(fileName)) {
        log_error(logger, "No existe archivo %s para truncar\n", fileName);
        return 0;
    }
    // fcb* fcbToTruncate = findFcbInGlobalOpenFiles(fileName);
    // if (fcbToTruncate == NULL) {
    //     log_warning(logger, "No se encuentra el archivo %s abierto por ningun proceso\n", fileName);
    //     return;
    // }
    uint32_t newBlocksNeeded = blocksPerBits(newSize);
    uint32_t usedBlocksFcb = blocksPerBits(getToMetadata(fileName, "TAMANIO_ARCHIVO"));
    int32_t initialBlock = getToMetadata(fileName, "BLOQUE_INICIAL");

    // if (usedBlocksFcb == newBlocksNeeded) {
    //     log_warning(logger, "No hace falta truncar %s, el archivo ya tiene los bloques necesarios\n", fileName);
    //     return 1;
    // }
    if(initialBlock == -1) {
        // No tiene bloques asignados, hay que asignarle y hay que buscar lugar para poder ubicar
        printf("Bloque inicial -1\n");
        int newInitBlock = canRelocate(fileName, newBlocksNeeded);
        if(newInitBlock >= 0) {
            printf("Bloque inicial %i\n", newInitBlock);
            initialBlock = newInitBlock;
            assignBit(bitmap, initialBlock);
            modifyMetadata(fileName, initialBlock , "BLOQUE_INICIAL");
        } else {
            printf("No hay lugar para truncar\n");
            return 0;
        }
    }
    uint32_t lastCurrentBlock = initialBlock + usedBlocksFcb - 1;
    if (usedBlocksFcb <= newBlocksNeeded) {
        printf("Sumamos Bloques\n");
        uint32_t blocksNeededToAdd = newBlocksNeeded - usedBlocksFcb;
        //printf("El FCB %s necesita %i bloques mas\n", fileName, blocksNeededToAdd);

        if(nextBlocksAvaiable(lastCurrentBlock + 1, blocksNeededToAdd)) {
            log_info(logger, "Todos los bloques siguientes de %s necesarios estan disponibles\n", fileName);
            for (int i = lastCurrentBlock + 1; i < lastCurrentBlock + blocksNeededToAdd + 1; i++) {
                assignBit(bitmap, i);
            }
            msyncBitmap();
            modifyMetadata(fileName, newSize , "TAMANIO_ARCHIVO");
        } else {
            // REUBICACION Y COMPACTACION
            //printf("De los bloques necesarios para %s, hay algunos que ya estan ocupados\n", fileName);
            // Tratamos primero de reubicar
            // int newInitBlock = canRelocate(fileName, newBlocksNeeded);
            // if(newInitBlock >= 0) {
            //     // Se puede reubicar
            //     log_info(logger, "Se reubica el archivo %s en el bloque %i\n", fileName, newInitBlock);
            //     realocate(fileName, newInitBlock, newSize);
            // } else {
            //     // No se puede reubicar, hay que tratar de compactar
            //     //printf("Hay que compactar\n");
            //     compaction();
            //     if (canRelocate(fileName, newBlocksNeeded)) {
            //         log_info(logger, "Se reubica el archivo %s en el bloque %i\n", fileName, newInitBlock);
            //         realocate(fileName, newInitBlock, newSize);
            //     } else {
            //         log_error(logger, "No se puede truncar %s\n", fileName);
            //         return 0;
            //     }
            // }

            // SOLO COMPACTACION
            printf("HAY QUE COMPACTAR\n");
            int newInitBlockTemp = canRelocateAtTheEnd(fileName, usedBlocksFcb);
            if (newInitBlockTemp >= 0) {
                realocate(fileName, newInitBlockTemp, usedBlocksFcb);
                compaction();
                uint32_t newUsedBlocksFcb = blocksPerBits(getToMetadata(fileName, "TAMANIO_ARCHIVO"));
                int32_t newInitialBlock = getToMetadata(fileName, "BLOQUE_INICIAL");
                uint32_t newLastBlock = newInitialBlock + newUsedBlocksFcb - 1;
                if(nextBlocksAvaiable(newLastBlock + 1, blocksNeededToAdd)) {
                    for (int i = newInitialBlock; i < newInitialBlock + newUsedBlocksFcb + blocksNeededToAdd; i++) {
                        assignBit(bitmap, i);
                    }
                } else {
                    log_error(logger, "No es posible truncar ni siquiera despues de compactar");
                    return 0;
                }
            }
        }
    } else {
        printf("Sacamos Bloques\n");
        uint32_t blocksNeededToTakeOut = usedBlocksFcb - newBlocksNeeded;
        //printf("El FCB %s necesita %i bloques menos\n", fileName, blocksNeededToTakeOut);
        for (int i = lastCurrentBlock; i > lastCurrentBlock - blocksNeededToTakeOut; i--) {
            deallocateBit(bitmap, i);
        }
        msyncBitmap();
        modifyMetadata(fileName, newSize , "TAMANIO_ARCHIVO");
    }
    return 1;
}

void fsDelete(char* fileName) {
    if (!metadataExist(fileName)) {
        log_warning(logger, "No es necesario eliminar por que %s no existe\n", fileName);
        return;
    }
    //printf("Eliminando FS %s\n", fileName);

    fcb* fcbOpen = findFcbInGlobalOpenFiles(fileName);
    if(fcbOpen != NULL) {
        log_info(logger, "El archivo %s se encuentra abierto, se cierra\n", fileName);
        closeFCB(fcbOpen, fcbOpen->pid);
    }

    char* path = pathMetadata(fileName);

    uint32_t initialBlock = getToMetadata(fileName, "BLOQUE_INICIAL");
    uint32_t fileSize = getToMetadata(fileName, "TAMANIO_ARCHIVO");

    uint32_t usedBlocks = blocksPerBits(fileSize);

    for (int i = initialBlock; i < initialBlock + usedBlocks; i++) {
        deallocateBit(bitmap, i);
    }
    msyncBitmap();

    if (remove(path) == 0) {
        log_info(logger, "Archivo %s eliminado exitosamente\n", path);
    } else {
        log_error(logger, "Error al eliminar el archivo %s\n", path);
    }
}

char* fsRead(char* fileName, uint32_t pointer, uint32_t dataSize) {
    if (!metadataExist(fileName)) {
        //printf("No se puede leer %s por que no existe\n", fileName);
        return NULL;
    }
    
    char* dataReaded = malloc(dataSize + 1);
    memset(dataReaded, 0, dataSize);
    dataReaded[dataSize] = '\0';
    uint32_t initialBit = getToMetadata(fileName, "BLOQUE_INICIAL") * getValue(BLOCK_SIZE) * 8;
    int dirPhysical = initialBit + pointer;
    for (int i = 0; i <= dataSize; i++) {
        char* binary = malloc(9);
        memset(binary, 0, 9);
        binary[9] = '\0';
        for (int j = 0; j < 8; j++) {
            int bit = (int)bitarray_test_bit(blocks, dirPhysical);
            sprintf(binary, "%s%i", binary, bit);
            dirPhysical++;
        }
        int ascii = binaryToInt8Bits(binary);
        char character = ascii;
        sprintf(dataReaded, "%s%c", dataReaded, character);
        free(binary);
    }
    return dataReaded;
}

void fsWrite(char* fileName, uint32_t pointer, char* data) {
    if (!metadataExist(fileName)) {
        log_info(logger, "No se puede escribir %s por que no existe\n", fileName);
        return;
    }
    //printf("Escribimos en %s, a partir del bit %u, la palabra %s\n", fileName, pointer, data);
    uint32_t initialBit = getToMetadata(fileName, "BLOQUE_INICIAL") * getValue(BLOCK_SIZE) * 8;
    int dirPhysical = initialBit + pointer;
    for (int i = 0; i < strlen(data); i++) {
        int ascii = data[i];
        char* binary = intToBinary8Bits(ascii);
        for (int j = 0; j < 8; j++) {
            int bit = binary[j] - '0';
            bit ? assignBit(blocks, dirPhysical) : deallocateBit(blocks, dirPhysical);
            dirPhysical++;
        }
        free(binary);
    }
    msyncBlocks();
}


int getFreeBlockIndex() {
    int i = 0;
    while (i < bitarray_get_max_bit(bitmap)) {
        if (!bitarray_test_bit(bitmap, i))
            return i;
        else
            i++;
    }
    return -1;
}



fcb* findFcbInGlobalOpenFiles(char* fileName) {
    bool compareFileName(void* element) {
        fcb* fcbCompare = (fcb*)element;
        return strcmp(fcbCompare->fileName, (char*)fileName) == 0;
    }
    fcb *fcbFinded = list_find(globalOpenFiles, compareFileName);
}

void createMetadata(char* fileName) {
    char* path = pathMetadata(fileName);
    // if (num_chars >= sizeof(path)) {
    //     printf("La salida ha sido truncada. Número de caracteres requeridos: %d\n", num_chars);
    //     free(path);
    //     return;
    // }

    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error al abrir o crear el archivo de configuración");
        free(path);
        return EXIT_FAILURE;
    }

    char* metadataContent = malloc(sizeof(char)*200);
    snprintf(metadataContent, 200, "BLOQUE_INICIAL=-1\nTAMANIO_ARCHIVO=0\n");
    ssize_t bytes_written = write(fd, metadataContent, strlen(metadataContent));

    if (close(fd) == -1) {
        log_error(logger, "Error al cerrar el archivo de configuración");
        return EXIT_FAILURE;
    }

    //printf("Metadata de %s creado con exito\n", fileName);
    free(metadataContent);
    free(path);
}

void modifyMetadata(char* fileName, uint32_t newSize, char* key) {
    char* path = pathMetadata(fileName);
    t_config* metadata = initConfig(path);
    char* value = malloc(sizeof(char)*10);
    snprintf(value, 10, "%u", newSize);
    config_set_value(metadata, key, value);
    if (!config_save(metadata)) {
        log_error(logger, "Error al guardar el archivo de configuración metadata %s\n", fileName);
    }
    config_destroy(metadata);
    free(value);
    free(path);
}


// t_file* createFile(char* fileName, t_config* metadata) {
//     t_file* newFile = malloc(sizeof(t_file));

//     newFile->name = malloc(sizeof(char) *50);
//     strcpy(newFile->name, fileName);
//     newFile->metadata = metadata;

//     return newFile;
// }


void assignBit(t_bitarray* bitarray, int blockIndex) {
    if (bitarray == NULL || bitarray->bitarray == NULL) {
        log_error(logger, "El bitmap no está inicializado correctamente\n");
        return;
    }

    bitarray_set_bit(bitarray, blockIndex);
}

void deallocateBit(t_bitarray* bitarray, int blockIndex) {
    if (bitarray == NULL || bitarray->bitarray == NULL) {
        log_error(logger, "El bitmap no está inicializado correctamente\n");
        return;
    }

    bitarray_clean_bit(bitarray, blockIndex);
}

int nextBlocksAvaiable(uint32_t startIndex, uint32_t blockCount) {
    for (int i = startIndex; i < startIndex + blockCount; i++) {
        if (bitarray_test_bit(bitmap, i)) {
            return 0; // Si algún bloque está ocupado, retorna 0 (no disponible)
        }
    }
    return 1; // Todos los bloques están disponibles
}

void msyncBitmap() {
    if (msync(bitmap->bitarray, bitmap->size, MS_SYNC) == -1) {
        log_error(logger, "Error en la sincronización de Bitmap con msync()\n");
    }
}

void msyncBlocks() {
    if (msync(blocks->bitarray, blocks->size, MS_SYNC) == -1) {
        log_error(logger, "Error en la sincronización de Blocks con msync()\n");
    }
}

int closeFCB(fcb* fbcToClose, uint32_t pid) {
    if(fbcToClose->pid == pid) {
        list_remove_element(globalOpenFiles, fbcToClose);
        deleteFcb(fbcToClose);
        log_info(logger, "Se cierra el archivo %s\n", fbcToClose->fileName);
        return 1;
    } else {
        log_warning(logger, "El archivo %s esta abierto por el proceso %i. El proceso %i no lo puede cerrar\n", fbcToClose->fileName, fbcToClose->pid, pid);
    }
    return 0;
}

int canRelocate(char* fileName, uint32_t necessaryBlocks) {
    int canReloc = 0;
    uint32_t initialBlock = getToMetadata(fileName, "BLOQUE_INICIAL");
    uint32_t fileSize = getToMetadata(fileName, "TAMANIO_ARCHIVO");

    uint32_t newInitialBlock;

    int i = 0;
    
    while (i < bitarray_get_max_bit(bitmap) && canReloc == 0) {
        int j = 0;
        if(i == initialBlock) {
            i = blocksPerBits(fileSize) + initialBlock;
        }
        for(j; j < necessaryBlocks; j++) {
            if (bitarray_test_bit(bitmap, i) || j == necessaryBlocks) {
                break;
            }
        }
        if(j == necessaryBlocks) {
            canReloc = 1;
            newInitialBlock = i;
        } else {
            i += j + 1;
        }
    }
    if(canReloc)
        return newInitialBlock;
    return -1;
}

int canRelocateAtTheEnd(char* fileName, uint32_t necessaryBlocks) {
    int canReloc = 0;
    uint32_t initialBlock = getToMetadata(fileName, "BLOQUE_INICIAL");
    uint32_t fileSize = getToMetadata(fileName, "TAMANIO_ARCHIVO");
    uint32_t fileBlocks = blocksPerBits(fileSize);

    uint32_t newInitialBlock;

    int i = bitarray_get_max_bit(bitmap) - 1; // Comenzamos desde el último bit
    
    while (i >= 0 && canReloc == 0) {
        int j = 0;
        
        // Verificar si i está en los bloques actuales del archivo
        if (i >= initialBlock && i < initialBlock + fileBlocks) {
            i = initialBlock - 1; // Saltar los bloques ocupados por el archivo actual
            continue;
        }
        
        for (j = 0; j < necessaryBlocks; j++) {
            if (bitarray_test_bit(bitmap, i - j) || (i - j) < 0) {
                break;
            }
        }
        
        if (j == necessaryBlocks) {
            canReloc = 1;
            newInitialBlock = i - necessaryBlocks + 1;
        } else {
            i -= (j + 1);
        }
    }
    
    if (canReloc)
        return newInitialBlock;
    return -1;
}

void realocate(char* fileName, uint32_t newInitialBlock, uint32_t newSize) {
    uint32_t initialBlock = getToMetadata(fileName, "BLOQUE_INICIAL");
    uint32_t fileSize = getToMetadata(fileName, "TAMANIO_ARCHIVO");
    uint32_t newBlocksNeeded = blocksPerBits(newSize);
    uint32_t usedBlocksFcb = blocksPerBits(getToMetadata(fileName, "TAMANIO_ARCHIVO"));
    uint32_t lastCurrentBlock = initialBlock + usedBlocksFcb - 1;
    uint32_t blocksNeededToAdd = newBlocksNeeded - usedBlocksFcb;

    char* buffer = malloc(fileSize + 1);
    buffer = fsRead(fileName, 0, fileSize);

    // fsTruncate(fileName, 0); // Al crear un archivo nuevo, ya se asigna un bloque, y el truncate 0 me deja el primer bloque asignado, aunque el tamaño de archovo sea 0, ...
    
    //printf("El FCB %s necesita %i bloques menos\n", fileName, blocksNeededToTakeOut);
    for (int i = lastCurrentBlock; i > lastCurrentBlock - usedBlocksFcb; i--) {
        deallocateBit(bitmap, i);
    }
    // bitarray_clean_bit(bitmap, initialBlock); // ...Por eso, el primer bloque anterior se setea en 0, ...

    // bitarray_set_bit(bitmap, newInitialBlock); // ...Y el nuevo primer bloque se setea en 1

    modifyMetadata(fileName, newInitialBlock, "BLOQUE_INICIAL");
    //fsTruncate(fileName, newSize);

    for (int i = newInitialBlock; i < newInitialBlock + blocksNeededToAdd + usedBlocksFcb; i++) {
        assignBit(bitmap, i);
    }
    msyncBitmap();
    modifyMetadata(fileName, newSize , "TAMANIO_ARCHIVO");

    fsWrite(fileName, 0, buffer);
    free(buffer);
}

void compaction() {
    log_info(logger, "COMPACTACIÓN");
    t_list* allMetadata = listMetadata();

    bool fileSizeComparison(void* elem1, void* elem2) {
        Metadata* metadata1 = (Metadata*) elem1;
        Metadata* metadata2 = (Metadata*) elem2;
        return (metadata1->block < metadata2->block);
    };

    list_sort(allMetadata, fileSizeComparison);

    int newPosition = 0;

    for (int i = 0; i < list_size(allMetadata); i++) {
        Metadata* metadata = list_get(allMetadata, i);
        if (metadata->block != newPosition) {
            realocate(metadata->fileName, newPosition, metadata->size);
        }
        free(metadata->fileName);
        free(metadata);
        newPosition += blocksPerBits(metadata->size);
    }
    list_destroy(allMetadata);
    //printf("Retraso Compactacion\n");
    usleep(getValue(RETRASO_COMPACTACION) *1000);
}

t_list* listMetadata() {
    t_list* allMetadata = list_create();
    DIR* dir;
    struct dirent* entry;
    int count = 0;

    if ((dir = opendir(path_metadata)) == NULL) {
        log_error(logger, "Error al abrir el directorio\n");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        //if (entry->d_type == DT_REG && isMetadata(entry->d_name)) {
        if (entry->d_type == DT_REG) {
            char filePath[256];
            snprintf(filePath, sizeof(filePath), "%s/%s", path_metadata, entry->d_name);

            // Verificar si el archivo existe
            if (access(filePath, F_OK) == 0) {
                Metadata* metadata = malloc(sizeof(Metadata));
                metadata->fileName = strdup(entry->d_name);
                metadata->size = getToMetadata(metadata->fileName, "TAMANIO_ARCHIVO");
                metadata->block = getToMetadata(metadata->fileName, "BLOQUE_INICIAL");
                list_add(allMetadata, metadata);
            }
        }
    }

    closedir(dir);
    return allMetadata;
}

int isMetadata(char* fileName) {
    char* ext = strrchr(fileName, '.');
    return (ext != NULL && strcmp(ext, ".txt") == 0);
}

char* fileNameWithoutExtens(char* fullFileName) {
    char* fileName = strdup(fullFileName);

    char* point = strrchr(fileName, '.');
    if (point != NULL) {
        *point = '\0';
    }
    return fileName;
}

uint32_t blocksPerBits(uint32_t bytes) {
    uint32_t blocks = (bytes + getValue(BLOCK_SIZE) - 1) / getValue(BLOCK_SIZE);
    return (blocks == 0) ? 1 : blocks;
}

uint32_t getToMetadata(char* fileName, char* key) {
    char* path = pathMetadata(fileName);
    t_config* metadata = config_create(path);
    if (metadata == NULL) {
        log_error(logger, "Error al inicializar la configuración para %s\n", path);
        free(path);
        return;
    }
    char* data = getConfigValue(metadata, key);
    int32_t dataInt = getValue(data);
    config_destroy(metadata);

    return dataInt;
}

char* pathMetadata(char* fileName) {
    int tamanioPath = 100;
    char* path = malloc(sizeof(char) *tamanioPath);
    snprintf(path, tamanioPath, "%s/%s", path_metadata, fileName);
    return path;
}

int metadataExist(char* fileName) {
    int fd;
    char* pathFile = malloc(sizeof(char)*100);
    sprintf(pathFile, "%s/%s", path_metadata, fileName);
    fd = open(pathFile, O_RDONLY);
    if (fd != -1) {
        // El archivo ya existe, se cierra el descriptor y se devuelve
        close(fd);
        free(pathFile);
        return 1;
    }
    free(pathFile);
    return 0;
}

char* uint32ToChar(uint32_t toTransform){
    char* buffer = malloc(11* sizeof(char));

    sprintf(buffer, "%u", toTransform);
    return buffer;
}

int directory_exists(char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0;
    }
    return S_ISDIR(statbuf.st_mode);
}

int create_directory(char* path) {
    if (mkdir(path, 0777) != 0) {
        if (errno != EEXIST) {
            return -1;
        }
    }
    return 1;
}

int ensure_directory(char* path) {
    if (!directory_exists(path)) {
        if (!create_directory(path)) {
            perror("Error creating directory");
            return -1;
        }
        //printf("Directory created: %s\n", path);
    } else {
        log_warning(logger, "Directory already exists: %s\n", path);
    }
    return 1;
}