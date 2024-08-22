#include "utils_memory.h"
#include "paginacion.h"

//como buscar una accion en particular
int initPagination(){
    memLengthInt = atoi(lengthMemory);
    pageLengthInt = atoi(lengthPage);
    mainMemory = malloc(memLengthInt);
    if(mainMemory == NULL)
    {
        log_info(log_memory , "No se pudo alocar memoria");
        return 0;
    }
    cantFrames = memLengthInt/pageLengthInt;
    log_info(log_memory , "La cantidad de frames es de: %d, cada uno con %d bytes en memoria principal", cantFrames, pageLengthInt);

    //crear bitArray
    dataForBitArray = asignarMemoriaBits(cantFrames);
    memset(dataForBitArray,0,cantFrames/8);//dividido 8 (8 bits)
    bitArrayOcupiedFrames = bitarray_create_with_mode(dataForBitArray, cantFrames/8, MSB_FIRST); //most significant bit first (de izquierda a derecha)
    pageTable = list_create();
    
    return 1;
}

int resize(int pid, int resizeSize)
{
    //me fijo si es menor o no.
    //fijarse ceil por si queda algo afuera
    //uint32_t + -1 si no pudo

    t_pageTable* arthursTable = getPageTable(pid);
    if(arthursTable == NULL)
    {
        //printf("\nNo habia tabla para el pid:%d\nProcedemos a usar playingGod\n",pid);
        int necessaryPages = ceil((double) resizeSize/ (double) pageLengthInt);
        log_info(log_memory, "\nNecesary pages: %d\n",necessaryPages);

        logNewPageTable(pid, necessaryPages);

        playingGod(pid, CREATION, necessaryPages);
    }else{
        //printf("\nSi existia una tabla para pid:%d\n",pid);
        int currentSize = list_size(arthursTable->pages);
        currentSize = currentSize*pageLengthInt;
        int sizeDifference = resizeSize-currentSize;
        //printf("\nCurrent size:%d\n",currentSize);
        //printf("\ndifference: %d\n",sizeDifference);
        if(sizeDifference==0)
        {
            log_info(log_memory, "\nYa tiene suficientes paginas para la cantidad de bytes\n");
        }else if(sizeDifference<0)
        {
            log_info(log_memory, "\nProcess reduction\n");
            int absReducedSize = abs(sizeDifference);
            int necessaryPages = ceil((double) absReducedSize/ (double) pageLengthInt);
            log_info(log_memory, "\nYou will now have %d pages\n",necessaryPages);
            freePagesFromTheEnd(arthursTable, necessaryPages); 
        }else
        {
            log_info(log_memory, "\nProcess enlargment\n");
            int necessaryPages = ceil((double) sizeDifference/ (double) pageLengthInt);
            log_info(log_memory, "\nYou will now have %d pages\n",currentSize+necessaryPages);
            if(isThereRoomInYourPage(necessaryPages))
            {
				t_list* auxList;  // VER SI SOLUCIONA MEMORY LEAK 16 BYTES
                char* aux;
                int bytes = sizeDifference;
                //printf("bytes:%d",bytes);
                aux = malloc(bytes);
                memset(aux,'*',bytes);
                t_list* pagesForInstructions = pagesOccupiedByInstructions(aux,pid);

                void showID(t_page* pag)
                {
                    log_info(log_memory, "La pagina que estas usando es: %d", pag->id);
                }

                log_info(log_memory, "Guardar las tareas del PCB %d en las paginas: ", pid);
                auxList = list_map(pagesForInstructions, (void*)showID);  // VER SI SOLUCIONA MEMORY LEAK 16 BYTES

                t_page* firstPage = list_get(pagesForInstructions, 0);
                int instructionAddress = calculateLogicalAddress(firstPage,0);

                t_pageTable* arthursTable = getPageTable(pid);
                addPagesToPCB(arthursTable, pagesForInstructions);

				list_destroy(auxList);  // VER SI SOLUCIONA MEMORY LEAK 16 BYTES
				free(aux);  // VER SI SOLUCIONA MEMORY LEAK 128 BYTES
            } else {
                log_error(log_memory, "No se puede hacer Resize, no hay espacio en memoria (Out Of Memory)\n");
                return 0;
            }
        }
    }
    return 1;
}

int writeInPagingMemory(char* instruction, int pid, int opSize, int physicalAddress)
{
    //char* algo = "AX 120";//esto no deberia estar harcodeado
    t_pageTable* arthursTable = getPageTable(pid);
    //printf("PAGINACIOOOOON ! instruction %s\n", instruction);
	logAccessToUserSpaceWrite(pid, physicalAddress, instruction);
    if(arthursTable!=NULL)
    {   
        ocupyPhysicalAddress((void*)instruction, opSize, physicalAddress, pid);
        return 1;
    }else
    {
        log_error(log_memory, "No habia tabla para ese PID\n");
        return -1;
    }
    
}


//void pointer para que sea texto lo que me devuelva o nulo en caso de error
char* readInPagingMemory(int pid, int opSize, int physicalAddress)
{
    t_pageTable* arthursTable = getPageTable(pid);
    if(arthursTable!=NULL)
    {
        char* limitedInstructions = leer_a_partir_de_direccion(pid, opSize, physicalAddress);
        /*int firstPage = ceill((physicalAddress)/pageLengthInt);
        printf("\nFirst page: %d\n",firstPage);
        t_list* pages = getPages(firstPage, opSize, arthursTable, pageLengthInt);
        char* instructions = getInstructionsFromPages(pages, arthursTable);
        char* limitedInstructions = malloc(opSize+1);
        memcpy(limitedInstructions,instructions,opSize);
        //memcpy(limitedInstructions, instructions+physicalAddress, opSize);
        //puts(limitedInstructions);
        limitedInstructions[opSize]='\0';
        printf("\n ins: %s\n", instructions);
        printf("\n ins-: %s\n", limitedInstructions);
        list_destroy(pages);
        logAccessToUserSpaceRead(pid, physicalAddress, opSize);*/
		logAccessToUserSpaceRead(pid, physicalAddress, limitedInstructions);
        return limitedInstructions;
    }else{
        log_error(log_memory, "La tabla no existia");
        return NULL;
    }
    
}


/*
t_pageTable* arthursTable = getPageTable(pid);
    /*int frame = physicalAddress->frame;
    int offset = physicalAddress->offset;
    
    int dir = frame + offset;
    if(arthursTable!=NULL)
    {
        logAccessToUserSpaceRead(pid, physicalAddress, opSize);

        //t_page* jimmyPage = findAFramesPage(frame);
        //t_list* pages = getPages(jimmyPage->id, opSize, arthursTable, pageLengthInt);
        t_list* pages = getPages(0, opSize, arthursTable, pageLengthInt);
        void showPageInfo(t_page* pag)
        {
            printf("\nLa pagina que estas accediendo es: %d\n", pag->id);
            printf("Frame: %d", pag->frame_ppal);
        }
        list_iterate(pages, (void *)showPageInfo);
        char* instructions = getInstructionsFromPages(pages, arthursTable);
        printf("\n ins: %s\n", instructions);
        list_destroy(pages);
        return instructions;

    }else
    {
        printf("No habia tabla para ese PID\n");
        return NULL;
    }
*/

char* getIndividualInstruction(int pid, int nextInstr, int* isLast){

    char* instructions = getInstructions(pid);
	char* instruction = separateInstructions(instructions, nextInstr, isLast); //ME DEVUELVE LA TAREA Y DICE SI ES LA ULTIMA

	return instruction;
}

char* separateInstructions(char* instructions, int offset, int* isLast){
	
	char** separateInstructions_S = string_split(instructions,"?");

	if(separateInstructions_S[offset] != NULL)
	{
		if(separateInstructions_S[offset + 1] == NULL)*isLast = 1;//Si la sig es NULL es la ultima
		else *isLast=0;//Si no es NULL, no es la ultima xD

		char* instruction = malloc(strlen(separateInstructions_S[offset])+1);
		strcpy(instruction, separateInstructions_S[offset]);
		free(instructions);
		freeStringArray(separateInstructions_S);
		return instruction;
		
	}
	else
	{
		free(instructions);
		freeStringArray(separateInstructions_S);
		return NULL; 
	}
			
}

char* getInstructionsFromPages(t_list* pages, t_pageTable* arthursTable)
{
    char* instructions;
    void* processInfo = copyPagesMP(pages);
    //pthread_mutex_unlock(&mutexCopiarPags);
    instructions = getInstructionsPages(processInfo, arthursTable->instructionSize);
    //printf("\nProcess Info: %d %p\n", sizeof(processInfo), processInfo);
    free(processInfo);	
    return instructions;
}

char* getInstructions(int pid){

    t_pageTable* arthursTable = getPageTable(pid);
    char* instructions;

    //HAY QUE SACAR LA INFORMACION DEL PROCESO QUE ESTA EN LAS PAGINAS DE LA TABLA BUSCADA
    t_list* pages = getPages(0, arthursTable->instructionSize, arthursTable, pageLengthInt);
    //por que busca por tamaño de instruccion definido en nla tabla?
    log_info(log_memory, "Instruction size: %d\n", arthursTable->instructionSize);
    void showPageInfo(t_page* pag)
    {
        log_info(log_memory, "\nLa pagina que estas accediendo es: %d\n", pag->id);
        log_info(log_memory, "Frame: %d\n", pag->frame_ppal);
    }
    list_iterate(pages, (void *)showPageInfo);
    //pthread_mutex_lock(&mutexCopiarPags);
    void* processInfo = copyPagesMP(pages);
    //pthread_mutex_unlock(&mutexCopiarPags);
    instructions = getInstructionsPages(processInfo, arthursTable->instructionSize);
    //printf("\nProcess Info: %d %p\n", sizeof(processInfo), processInfo);
    free(processInfo);
	list_destroy(pages);
	
    return instructions;
}

t_list* getPages(int nfirstPage, int size, t_pageTable* arthursTable, int filling)
{
	
	t_list* ocuppiedPages = list_create();
	t_page* firstPage = list_get(arthursTable->pages, nfirstPage);
    
	//pthread_mutex_lock(&mutexPags);
	//firstPage->uso = 1; //COMO ACCEDI A LA PAG -> MODIFICO EL USO
	//firstPage->tiempo_uso = obtenerTiempo(); //COMO ACCEDI -> ACTUALIZO EL TIEMPO DE USO
	//pthread_mutex_unlock(&mutexPags);
	//SI EL TAMANIO DEL ELEMENTO ES MENOR O IGUAL A LO QUE ESTA OCUPADO EN LA PAGINA
	if(size <= filling){   //filling seria lo que le falta para completar la pagina, si lo que quiero es menor a este, solo uso una pag
		list_add(ocuppiedPages, firstPage);
		
	}else{	//SI NO -> EL RESTO VA A ESTAR EN LA PAGINA QUE LE SIGUE 
		int nextPageNmbr = nfirstPage + 1;
		
		t_list* otherPages = getPages(nextPageNmbr, size - filling, arthursTable, pageLengthInt);
		
		//AGREGAMOS LAS PAGINAS OCUPADAS 
		list_add(ocuppiedPages, firstPage); 
        list_add_all(ocuppiedPages, otherPages);
		
		list_destroy(otherPages);
	}
	return ocuppiedPages;
}

void* copyPagesMP(t_list* pages){
	
	void* algo = malloc(pageLengthInt * list_size(pages));
    copyPages(pages, algo);

    return algo;

}

void copyPages(t_list* pageList, void* algo){
	
	int offset = 0;
	
	void copyIn(t_page* jimmyPage){
		copyPageAfter(jimmyPage, algo, &offset);
	}
	
	list_iterate(pageList, (void*)copyIn);	
}

void copyPageAfter(t_page* jimmyPage, void* algo, int* offset){
	
	int base = jimmyPage->frame_ppal * pageLengthInt;
	
	//pthread_mutex_lock(&mutexMemoria);
	memcpy(algo + *offset, mainMemory + base, pageLengthInt);
	//pthread_mutex_unlock(&mutexMemoria);
	
	*offset += pageLengthInt;
}

char* getInstructionsPages(void* processInfo , int size ){
    char* instructions = malloc(size);   
    memcpy(instructions, processInfo, size);
    return instructions;
}

//int logicalAddress = (page->id * pageLengthInt) + offset;
/*offset_in_page = logical_address % page_size;
page_table_index = logical_address / page_size;
Then get the physical address of the page from the page table:

physical_address_of_page = page_table[page_table_index].physical_address = page_table[page_table_index].frame * page_size;
Then add the offset within the page to get the final physical address:

physical_address = physical_address_of_page + offset_in_page;*/

int playingGod(int pid, godMode mode, int numberPages)
{   
    int bytes;
    if(mode == DESTRUCTION)
    {
        //printf("Playing the all destroyer I see...\n");
        t_pageTable* arthursTable = getPageTable(pid);
        freePagesFromTheEnd(arthursTable, numberPages);// arthursTable -> pages -> elements_count); 
    }else if(mode==CREATION)
    {
        //printf("Playing the all creator I see...\n");
        if(isThereRoomInYourPage(numberPages))
        {
            t_pageTable* aTable = getPageTable(pid);
            if(aTable != NULL)
            {
                log_info(log_memory, "La tabla para ese PID ya existe, por favor usar un resize\n");
                return 0;
            }else
            {
                //char* aux = "SUM B9 B10 ? RESIZE A2 4 ? SUM A2 30";
                char* aux;
                //bytes = strlen(aux) + 1;
                bytes = (numberPages) * pageLengthInt;
               // printf("BYTES: %d",bytes);
                aux = malloc(bytes);
                memset(aux,'*',bytes);
                t_list* pagesForInstructions = pagesOccupiedByInstructions(aux,pid);
				t_list* auxList;  // VER SI SOLUCIONA MEMORY LEAK 144 BYTES

                void showID(t_page* pag)
                {
                    log_info(log_memory, "La pagina que estas usando es: %d\n", pag->id);
                }
                log_info(log_memory, "Guardar las tareas del PCB %d en las paginas: \n", pid);
                auxList = list_map(pagesForInstructions, (void*)showID);  // VER SI SOLUCIONA MEMORY LEAK 144 BYTES

                t_page* firstPage = list_get(pagesForInstructions, 0);
                int instructionAddress = calculateLogicalAddress(firstPage,0);

                t_pageTable* arthursTable = createPageTable(pid, bytes);
                addPagesToPCB(arthursTable, pagesForInstructions);
                int numberOfPages = list_size(arthursTable->pages);
                idGlobal-=numberOfPages;
                //printf("\nNunmber of pages: %d\nIdGlobal: %d\n",numberOfPages,idGlobal);
                arthursTable->lastUsedPageID = numberOfPages;

                //pcb* miPCB = malloc(sizeof(pcb));
                //miPCB -> pid = pid;
                //miPCB -> quantum = instructionAddress;
                
                //savePCBPagination(miPCB);
                //free(miPCB);

				list_destroy(auxList);  // VER SI SOLUCIONA MEMORY LEAK 144 BYTES
				free(aux);  // VER SI SOLUCIONA MEMORY LEAK 128 BYTES

                return 1;
            }
            
        }else{
            return 0;
        }
    }
    
}

void savePCBPagination(pcb* PCB){

    int offset=0;
    t_pageTable* table = getPageTable(PCB->pid);
    t_page* page = saveSomething(PCB, 8, table, &offset);

    log_info(log_memory, "Guarde la PCB %d en las pagina %d y su desplazamiento es %d\n", table->pid, page->id, offset);
	
    int logicalAddress = calculateLogicalAddress(page, offset);
    //printf("The logical address of the PCB is: %d\n", logicalAddress);
	table->pcbAddr = logicalAddress;

}

t_pageTable* getPageTable(int pid)
{
    t_pageTable* arthursTable;

    int sameID(t_pageTable* table)
    {
        return (table -> pid == pid);
    }

    //pthread_mutex_lock(&mutexListaTablas);
	arthursTable = list_find(pageTable, (void*)sameID);
	//pthread_mutex_unlock(&mutexListaTablas);

	return arthursTable;	
}

t_page* saveSomething(void* algo, int size, t_pageTable* table, int* offset)
{
    t_list* pages;
    t_page* firstPage;

    if(pagesHaveRoom(table))
    {
        t_list* availablePages = findAvailablePages(table);
        firstPage = list_get(availablePages,0);
        pages = fillPageSpace(algo, firstPage, size, offset, table->pid);
    }
}

t_list* fillPageSpace(void* structure, t_page* pageToBeOccupied, int size, int* offset, int pid)
{
    t_list* usedPages;

    if(pageToBeOccupied->tamanioDisponible >= size){       //ENTRA EN LA PAGINA JUSTO O LE SOBRA ESPACIO
        int frame = pageToBeOccupied->frame_ppal;
        *offset = pageLengthInt - pageToBeOccupied->tamanioDisponible;   //EMPIEZO A OCUPAR A PARTIR DE ESTO

        ocupyPagingMemory(structure, size, frame, *offset);
    
        pageToBeOccupied->tamanioDisponible -= size; //LE RESTO LO QUE ACABO DE OCUPAR
        
        usedPages = list_create();
        
    }else{                                              //SI NO ENTRA EN LA PAGINA, OCUPO UN POOC DE ESA Y CREO UNA NUEVA
        int freePageSpace = pageToBeOccupied->tamanioDisponible;

        t_list* nada = fillPageSpace(structure, pageToBeOccupied, freePageSpace, offset, pid);    //RECURSIVA, LLENO LO QUE PUEDO
        list_destroy(nada);
        //GUARDO LO QUE QUEDA EN UNA PAGINA NUEVA

        usedPages = saveInPage(structure+freePageSpace, size-freePageSpace,pid);
    }

    return usedPages;
}

int pagesHaveRoom(t_pageTable* table){

    t_list* pages= findAvailablePages(table);

    if(list_is_empty(pages)){//NO HAY PAGINAS CON ESPACIO DISPONIBLE 
        list_destroy(pages);
        return 0;
    }
    else 
    {
        list_destroy(pages);
        return 1;    	
    }
}

t_list* findAvailablePages(t_pageTable* table){

    t_list* availablePages;
    t_list* tablePages = table->pages;

    int roomInAPage(t_page* page){
        return (page->tamanioDisponible > 0);
    } 

    availablePages = list_filter(tablePages, (void*)roomInAPage);

    return availablePages;
}

int calculateLogicalAddress(t_page* page, int offset)
{
    int logicalAddress = (page->id * pageLengthInt) + offset;
    return logicalAddress; 
}

void addPagesToPCB(t_pageTable* arthursTable, t_list* instructionPages)
{
    addPagesToTable(arthursTable, instructionPages);
    list_destroy(instructionPages);
}

void addPagesToTable(t_pageTable* arthursTable, t_list* pages)
{
    list_add_all(arthursTable -> pages, pages);
}

char* asignarMemoriaBits(int bits)//recibe bits asigna bytes
{
	char* aux;
	int bytes;
	bytes = bitsToBytes(bits);
	printf("BYTES: %d\n", bytes);
	aux = malloc(bytes);
	memset(aux,0,bytes);
	return aux; 
}

int isThereRoomInYourPage(int necessaryPages){
    int availableFrames = pageAvailability(); 
    if((availableFrames) >= necessaryPages)
        return 1;
    else
        return 0;
}

int pageAvailability(){
	int freeSpaces = 0;
	int offset = 0;
    while(offset < cantFrames){
        //pthread_mutex_lock(&mutexBitmapMP);
        if(bitarray_test_bit(bitArrayOcupiedFrames,offset) == 0)
            freeSpaces++; 
        //pthread_mutex_unlock(&mutexBitmapMP);
        offset++;
    }
	return freeSpaces;	
}

t_list* pagesOccupiedByInstructions(char* instructions,int pid){

    t_list* pagesOccupiedByInstructions;
    
    int size = strlen(instructions);

    pagesOccupiedByInstructions = saveInPage(instructions, size,pid);
    
    return pagesOccupiedByInstructions; //DEVUELVE LA O LAS PAGINAS EN DONDE SE GUARDARON LAS TAREAS   
}

t_list* saveInPage(void* algo, int size, int pid){ //GUARDA EN PAGINAS NUEVAS -> VACIAS 
    
    t_list* pagesOccupiedByInstructions = list_create();
    //BUSCO FRAMES LIBRES
    t_list* unocupiedFrames = findFreeFrames();

    int pageSize = pageLengthInt;

    if(list_is_empty(unocupiedFrames))
    { //NO HAY FRAMES LIBRES EN MEMORIA PPAL 

        //printf("No hay espacio en la memoria y... no hay virtual asi que... cagaste\n");
        log_error(log_memory, "No hay espacio en la memoria principal");
    
    }else
    { //HAY FRAMES LIBRES EN PPAL
        if(size <= pageSize)
        {   //SI LO QUE TENGO QUE GUARDAR ENTRA EN UNA PAGINA
        
            t_frame* frame = list_get(unocupiedFrames,0); // AGARRO EL PRIMERO QUE ENCUENTRO EN LA LISTA DE LIBRES
            saveInPagingMemory(algo, size, frame,pid);
        
            t_page* page = pageCreate(frame, (pageSize - size),pid); //SI LAS TAREAS OCUPAN MENOS QUE LA PAG ME VAN A SOBRAR BYTES
            list_add(pagesOccupiedByInstructions, page);
        }   
        else{  //SI NECESITO MAS PAGINAS
        	
            int remnant = size - pageSize;        //lo que no me entra en la primer pagina
        
            t_list* page = saveInPage(algo, pageSize,pid);   //GUARDO LO PRIMERO
            t_list* pageRemnant = saveInPage(algo + pageSize, remnant,pid);
        
            list_add_all(pagesOccupiedByInstructions,page);
		    list_add_all(pagesOccupiedByInstructions,pageRemnant);

            list_destroy(page);
            list_destroy(pageRemnant);
        }
    }
    deleteList(unocupiedFrames);
    return pagesOccupiedByInstructions;
}

t_list* findFreeFrames(){
    t_list* freeFrames = list_create();
    int base = 0;
    //pthread_mutex_lock(&mutexBitmapMP);
    while(base < cantFrames){
        if(bitarray_test_bit(bitArrayOcupiedFrames, base) == 0){ //VE LOS 0 EN EL BITMAP
            t_frame* unFrame = malloc(sizeof(t_frame));//Va aca adentro porque pertence al loop
            unFrame->id = base;
            list_add(freeFrames, unFrame);
        }
    base++; 
    }
	//pthread_mutex_unlock(&mutexBitmapMP);	
    return freeFrames; 
}

void saveInPagingMemory(void* algo, int size,t_frame* unFrame,int pid){

    ocupyFrame(unFrame);
    ocupyPagingMemory(algo, size, unFrame->id, 0); //LE PASO 0 PORQUE ES UNA PAGINA VACIA
    
}

void ocupyFrame(t_frame* unFrame){
    //pthread_mutex_lock(&mutexBitmapMP);
    bitarray_set_bit(bitArrayOcupiedFrames, (unFrame->id));
    //pthread_mutex_unlock(&mutexBitmapMP);	
}

void freeAFrame(int frame){
    //pthread_mutex_lock(&mutexBitmapMP);
    bitarray_clean_bit(bitArrayOcupiedFrames, frame);
    //pthread_mutex_unlock(&mutexBitmapMP);	
}

void ocupyPagingMemory(void* algo, int size, int idFrame, int offset){

    //BASE DEL FRAME = ID*TAMANIO_PAGINA + DESPLAZAMIENTO(dentro de la pagina) -> EL ID INDICA EL NRO DE FRAME

    int base = ((idFrame * pageLengthInt) + offset);
    //pthread_mutex_lock(&mutexMemoria);
    memcpy(mainMemory + base, algo, size);
    //pthread_mutex_unlock(&mutexMemoria);

}

void ocupyPhysicalAddress(void* algo, int size, int address, int pid)
{
    //memcpy(mainMemory + address,algo,size);
    //printf("Write Address: %p",mainMemory+address);
    uint32_t bytes_copiados = 0;
    uint32_t bytes_disp_frame = getValue(lengthPage) - obtener_desplazamiento_pagina(address);
	if (bytes_disp_frame >= size)
	{	
		//printf("\nMemoria antes de la escritura:\n");
		//mem_hexdump(mainMemory, 64);
		memcpy(mainMemory + address, algo, size);
		bytes_copiados += size;
		//printf("\nMemoria despues de la escritura:\n");
		//mem_hexdump(mainMemory, 64);
		//log_info(logger_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño %d", pid, direccion_fisica, bytes_a_copiar_original);

		return;
	}
	else
	{
		memcpy(mainMemory + address, algo, getValue(lengthPage));
		bytes_copiados += getValue(lengthPage);
		// termina una pagina
		size = size - getValue(lengthPage);
		int cant_paginas_restantes = ceil((float)size / getValue(lengthPage));
		uint32_t direccion_fisica_nueva = address;
		while (cant_paginas_restantes > 1)
		{
			t_page* pagina_siguiente = obtener_pagsig_de_dirfisica(direccion_fisica_nueva, pid);
			direccion_fisica_nueva = recalcular_direccion_fisica(pagina_siguiente);
			//printf("\nMemoria antes de la escritura:\n");
			//mem_hexdump(mainMemory+direccion_fisica_nueva, getValue(lengthPage));
			memcpy(mainMemory + direccion_fisica_nueva, algo + bytes_copiados, getValue(lengthPage));
			//printf("\nMemoria despues de la escritura:\n");
			//mem_hexdump(mainMemory+direccion_fisica_nueva, getValue(lengthPage));
			bytes_copiados += getValue(lengthPage);
			size = size- getValue(lengthPage);
			cant_paginas_restantes = ceil((float)size / getValue(lengthPage));
			
		}
		t_page* pagina_siguiente = obtener_pagsig_de_dirfisica(direccion_fisica_nueva, pid);
		direccion_fisica_nueva = recalcular_direccion_fisica(pagina_siguiente);
		//printf("\nMemoria antes de la escritura:\n");
		//mem_hexdump(mainMemory, 64);
		memcpy(mainMemory + direccion_fisica_nueva, algo + bytes_copiados, size);
		//printf("\nMemoria despues de la escritura:\n");
		//mem_hexdump(mainMemory, 64);
	}

}

//(create) Tabla de paginacion por proceso
t_pageTable* createPageTable(int pid, int size){
	
	t_pageTable* arthursTable = malloc(sizeof(t_pageTable));
	
	arthursTable->pid = pid;
	arthursTable->instructionSize = size;
	arthursTable->pages = list_create();
    arthursTable->lastUsedPageID = 0;
	
	//pthread_mutex_lock(&mutexListaTablas);
	list_add(pageTable, arthursTable);
	//pthread_mutex_unlock(&mutexListaTablas);
	
	return arthursTable;
}

//(create) Pagina individual
t_page* pageCreate(t_frame* frame, int espacioDisp,int pid){

    t_page* jimmyPage = malloc(sizeof(t_page));
    //printf("\npid: %d\n",pid);

    t_pageTable* arthursTable = getPageTable(pid);
    if(arthursTable!=NULL)
    {
        jimmyPage-> id = arthursTable->lastUsedPageID;
        arthursTable->lastUsedPageID += 1;
    }else
    {
        //printf("NO existe tabla para el pid: %d",pid);
        jimmyPage-> id = generarId();
    }
    //jimmyPage-> id = generarId();
	jimmyPage->frame_ppal = frame->id;
	jimmyPage->tamanioDisponible = espacioDisp;
	jimmyPage->fragInterna = 0;
	jimmyPage->presencia = 1;//esta en mp
	jimmyPage->modificado = 1;//Lo pongo en 1 asi se carga en disco
	jimmyPage->lock = 0;
	jimmyPage->uso = 0;
	jimmyPage->tiempo_uso = 1;

    return jimmyPage;
}

int generarId(){
	
	//pthread_mutex_lock(&mutexIdGlobal);
	int t = idGlobal;
	idGlobal++;
	//pthread_mutex_unlock(&mutexIdGlobal);
	return t;
}

void deleteList(t_list* list){
	list_destroy_and_destroy_elements(list, (void*)deleteAlgo);
}

void deleteAlgo(void* algo){
	
	free(algo);
	
}

//Funciones de control
void dumpPaginacion(void){

    showPageFrames();

}

void showPageFrames(){ 

    for(int i = 0; i <20; i++){//cantFRames
        showFrame(i);
    }
}

void showFrame(int nFrame){

    t_page* framePage = findAFramesPage(nFrame);

    //Muestro los datos que quiero

    if(framePage != NULL){//El frame esta ocupado

        int pid = findPID(framePage);

        log_info(log_memory, "Marco: %d    Estado:Ocupado        Proceso: %d    Pagina: %d \n",nFrame, pid, framePage->id );

        //printf("\n\n");
    }
    else{//El frame no esta ocupado

        log_info(log_memory, "Marco: %d    Estado:Libre                 Proceso:  -    Pagina:  - \n",nFrame);
        //printf("\n\n");
    }
}

//El módulo deberá responder el número de marco correspondiente a la página consultada.
int getFrame(int pid, int pageNumber)
{
    t_page* page;

    t_pageTable* arthursTable = getPageTable(pid);

    if(arthursTable != NULL)
    {
        int frame;

        if(list_size(arthursTable->pages) > pageNumber)
        {
            page = list_get(arthursTable->pages, pageNumber);
            log_info(log_memory, "La pagina %d tiene ID: %d\n", pageNumber, page -> id);
            log_info(log_memory, "La pagina %d tiene marco principal: %d\n", pageNumber, page -> frame_ppal);
            return page->frame_ppal;
        }   
        else
        {
            log_error(log_memory, "La tabla no tiene pagina %d\n", pageNumber);
            return -1;
        }
    }else
    {
        log_error(log_memory, "No hay tabla para ese PID\n");
        return -1;
    }
    
}

t_page* findAFramesPage(int nFrame){

    t_page* page;

    t_list* pages = findAllPages();

    int hasAFrame(t_page* pag){
        return (pag->frame_ppal) == nFrame;
    }
    
    if(list_find(pages, (void*)hasAFrame) != NULL){

        page = list_find(pages, (void*)hasAFrame);
        list_destroy(pages);
        return page;
    }else{
            list_destroy(pages);
            return NULL;
    }
}

t_list* findAllPages(){
	
	t_list* pages = list_create();
	
	//BUSCO TODAS LAS PAGINAS
	
	void tablePages(t_pageTable* arthursTable)
	{	
		list_add_all(pages, arthursTable->pages);		
	}
	//pthread_mutex_lock(&mutexListaTablas);
	list_iterate(pageTable, (void*)tablePages);
	//pthread_mutex_unlock(&mutexListaTablas);
	//FILTRO LAS QUE TENGAN EL BIT DE PRESENCIA EN 1 => ESTAN EN MP
	
	int presenceBit(t_page* unaPag)
	{
		return (unaPag->presencia == 1);
	}
	
	t_list* filteredPages = list_filter(pages, (void*)presenceBit);
	
	list_destroy(pages);
	return filteredPages;
}

int findPID(t_page* page)
{
    int hasAPage(t_pageTable* table)
    {
        int sameID(t_page* pag){
            return pag->id == page->id && pag->frame_ppal == page->frame_ppal;
        }
        return list_any_satisfy(table->pages, (void*)sameID);
    }
    //pthread_mutex_lock(&mutexListaTablas);
    t_pageTable* table = list_find(pageTable, (void*)hasAPage);
    //pthread_mutex_unlock(&mutexListaTablas);
    return table->pid;
}

void killAPage(t_page* jimmyPage){
	
	//LA SACO DEL BITMAP
	freeAFrame(jimmyPage->frame_ppal);
	
}


void freePages(t_list* pages, int size, t_pageTable* arthursTable, int base, int filling){

	if(list_size(pages) > 1){ //OCUPO MAS DE UNA PAGINA
			
		if((base % pageLengthInt) == 0){	//SI ESTOY AL PRINCIPIO DE UN FRAME, LO LIBERO ENTERO
			freeAPage(list_get(pages,0), arthursTable);	//Como tengo mas de una pagina, y empiezo desde el inicio de la misma, 100% seguros que ocupa toda la misma
			list_remove(pages,0);
			freePages(pages, size - pageLengthInt, arthursTable, base + pageLengthInt,pageLengthInt);
		}else{ 												//SI HAY OTRAS COSAS EN LA PAGINA
			t_page* primerPag = list_get(pages,0);
			primerPag->fragInterna += filling; 		//el cachito que ocupaba de la pagina queda como frag interna
			if(primerPag->fragInterna == pageLengthInt){//Si el espacio libre mas lo que tengo mas la frag interna es un frame, NO HAY NADA MAS EN LA PAG Y LA PUEO LIBERAR
				freeAPage(primerPag, arthursTable);					
			}
			list_remove(pages,0);
			freePages(pages, size - filling, arthursTable, base + filling, pageLengthInt);//Muevo la base el sobramte, que es el cachito que ocupa
		}
	}else{ 													//Aca tengo que ver si ocupa toda la pagina o solo un cachito, pero nunca se sale de la misma
		t_page* unicaPag = list_get(pages,0);
		
		if(unicaPag->fragInterna + unicaPag->tamanioDisponible + size == pageLengthInt) {//SI LA SUMA DE TODO ESO DA IGUAL AL TAM DE UNA PAGINA, ENTONCES SIGNIFICA QUE NO HAY NADA MAS Y LA PUEDO BORRAR
			freeAPage(unicaPag, arthursTable);		//LIBERO LA PAGINA			
		}
		else unicaPag->fragInterna += size; //LO Q "BORRE" (EL SIZE) QUEDA COMO FRA INTERNA 
		//En el resto de los casos no me importa liberar la pagina, porque la ocupa otra cosa
	}
}
void freeAPage(t_page* jimmyPage, t_pageTable* arthursTable){
	
	//ANULO LA PAGINA
	jimmyPage->id = -1;//Asi no la encuentra nadie
	
	//LA SACO DEL BITMAP
	freeAFrame(jimmyPage->frame_ppal);
	
	//ANULO EL FRAME DE LA PAGINA 
	jimmyPage->frame_ppal = -1;
		
	jimmyPage->tamanioDisponible = 0;
}
void freePagesFromTheEnd(t_pageTable* arthursTable, int numberPages){
	int lastsIndex = arthursTable -> pages -> elements_count - 1;
    while(numberPages > 1)
    {
        t_page* lastPage = list_get(arthursTable -> pages, lastsIndex);
        //printf("\nlast page id: %d and index %d\n", lastPage->id, lastsIndex);
        freeAFrame(lastPage->frame_ppal);
        list_remove(arthursTable -> pages, lastsIndex);
        arthursTable->lastUsedPageID -=1;
        lastsIndex -=1;
        numberPages -=1;
        //idGlobal--;
    }
}

void freePagingMemory(){
	
	bitarray_destroy(bitArrayOcupiedFrames);
	free(dataForBitArray);
	//pthread_mutex_lock(&mutexListaTablas);
	list_destroy_and_destroy_elements(pageTable, (void*)deletePageTable);
	//pthread_mutex_unlock(&mutexListaTablas);
}

void deletePageTable(t_pageTable* arthursTable){
    int pid = arthursTable->pid;
    int numPages = list_size(arthursTable->pages);

    logDestroyPageTable(pid, numPages);

	deleteList(arthursTable->pages);
	free(arthursTable);
}



void freeStringArray(char** stringArray){
	
	int size = sizeStringArray(stringArray);
	
	for(int i = 0; i< size; i++){
		free(stringArray[i]);
	}
	
	free(stringArray);
	
}

int sizeStringArray(char** a)
{	
	int i = 0;
	while(a[i] != NULL)
	{
		i++;
	}
	return i;
}

int obtener_numero_marco(int direccion_fisica)
{
	return floor(direccion_fisica / getValue(lengthPage));
}

int obtener_desplazamiento_pagina(int direccion_fisica)
{
	int numero_marco = obtener_numero_marco(direccion_fisica);
	return direccion_fisica - numero_marco * getValue(lengthPage);
}

t_page *obtener_pagsig_de_dirfisica(uint32_t direccion_fisica, uint32_t pid)
{
    
	uint32_t marco_actual = obtener_numero_marco(direccion_fisica);
    t_list *pages = getPageTable(pid)->pages;
	int i = 0;
	int pagina_encontrada = 0;
	while (i < list_size(pages) && pagina_encontrada != 1)
	{
		t_page *pagina = list_get(pages, i);
		if (marco_actual == pagina->frame_ppal)
		{
			pagina_encontrada = 1;
			return pagina = list_get(pages, i + 1);
		}
		i++;
	}
	return NULL;
}
uint32_t recalcular_direccion_fisica(t_page *pagina)
{
	return pagina->frame_ppal * getValue(lengthPage); // retorna la direccion_fisica
}

char* leer_a_partir_de_direccion(int pid, int opSize, int physicalAddress)
{
	
    char* valor_a_leer = calloc(1, (size_t)opSize + 1);
	int bytes_usados = 0;
	int desplazamiento = obtener_desplazamiento_pagina(physicalAddress);
	int bytes_restantes = opSize; 

	// si el desplazamiento es distinto de cero leemos hasta terminar la pagina
	if (desplazamiento != 0){
		bytes_usados = getValue(lengthPage) - desplazamiento;
		//printf("\nVariable antes de la lectura:\n");
		//mem_hexdump(valor_a_leer, bytes_a_leer);
		memcpy(valor_a_leer, mainMemory+physicalAddress, bytes_usados);
		//printf("\nVariable despues de la lectura:\n");
		//mem_hexdump(valor_a_leer, bytes_a_leer);
		
		bytes_restantes = opSize - bytes_usados;
		
		// caso en donde leyendo de la primer pagina ya completamos la solicitud
		if (bytes_restantes == 0) {
			//log_info(logger_memoria, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d", pid, direccion_fisica, bytes_a_leer);
			return valor_a_leer;
		}
	} else {
		// en este caso tenemos que leer de una sola pagina pero desde el comienzo - CON PROBABILIDAD DE QUE NO SEA SUFICIENTE -

		// nos fijamos si lo que hay que leer es menor o igual a una pagina para asegurar que leemos y completamos la solicitud
		if (opSize <= getValue(lengthPage)){
			//printf("\nVariable antes de la lectura:\n");
			//mem_hexdump(valor_a_leer, bytes_a_leer);
			memcpy(valor_a_leer,  mainMemory+physicalAddress, opSize);
			//printf("\nVariable despues de la lectura:\n");
			//mem_hexdump(valor_a_leer, bytes_a_leer);

			valor_a_leer[opSize] = '\0';
			//log_info(logger_memoria, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d", pid, direccion_fisica, bytes_a_leer);
			return valor_a_leer;
		} else {
			// en este caso leemos una pagina completa y actualizamos los bytes usados
			//printf("\nVariable antes de la lectura:\n");
			//mem_hexdump(valor_a_leer, bytes_a_leer);
			memcpy(valor_a_leer,  mainMemory+physicalAddress, getValue(lengthPage));
			//printf("\nVariable despues de la lectura:\n");
			//mem_hexdump(valor_a_leer, bytes_a_leer);

			bytes_usados += getValue(lengthPage);
			bytes_restantes = opSize - getValue(lengthPage);
		}
	}

	// una vez completada la primera pagina seguimos leyendo la cantidad de bytes de una pagina completa
	// hasta que no se pueda mas

	t_page* pagina_siguiente = NULL;
	uint32_t direccion_fisica_nueva = physicalAddress;

	while (bytes_restantes != 0 && bytes_restantes >= getValue(lengthPage)){
		pagina_siguiente = obtener_pagsig_de_dirfisica(direccion_fisica_nueva, pid);
		direccion_fisica_nueva = recalcular_direccion_fisica(pagina_siguiente);
		//printf("\nVariable antes de la lectura:\n");
		//mem_hexdump(valor_a_leer, bytes_a_leer);
		memcpy(valor_a_leer+bytes_usados, mainMemory+direccion_fisica_nueva, getValue(lengthPage));
		//printf("\nVariable despues de la lectura:\n");
		//mem_hexdump(valor_a_leer, bytes_a_leer);
		
		bytes_usados += getValue(lengthPage);
		bytes_restantes -= getValue(lengthPage);
	}

	// si sobraron bytes leemos esa cantidad 

	if (bytes_restantes != 0){
		pagina_siguiente = obtener_pagsig_de_dirfisica(direccion_fisica_nueva, pid);
		direccion_fisica_nueva = recalcular_direccion_fisica(pagina_siguiente);
		//printf("\nVariable antes de la lectura:\n");
		//mem_hexdump(valor_a_leer, bytes_a_leer);
		memcpy(valor_a_leer+bytes_usados, mainMemory+direccion_fisica_nueva, bytes_restantes);
		//printf("\nVariable despues de la lectura:\n");
		//mem_hexdump(valor_a_leer, bytes_a_leer);
	}

	// si ya tenia el \0 no quiero agregarlo de nuevo por lo que achico al tamaño original
	if(valor_a_leer[opSize-1] != '\0'){
		valor_a_leer = realloc(valor_a_leer, opSize+1);
		valor_a_leer[opSize] = '\0';
	}
		
	//log_info(logger_memoria, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d", pid, direccion_fisica, bytes_a_leer);
	return valor_a_leer;
}



/*
		A modo de prueba, ejecutaré secuencialmente las operaciones de memoria pedidas como requisitos minimos
		en el trabajo practico
	
	int pid = 6;
	int size = 3; 
	int mode = CREATION;
	int r = playingGod(pid, mode, size);

	pid = 7;
	size = 3;
	mode = CREATION;
	r = playingGod(pid, mode, size);

	pid = 6;
	size = 32;
	r = resize(pid, size);

	int pidPrueba = 6;
	int numeroPaginaPrueba = 4;
	int marcoDeUnaPaginaYUnID = getFrame(pidPrueba,numeroPaginaPrueba);

	printf("\nEl marco de la pagina consultada (%d) del proceso (%d) es: %d\n", numeroPaginaPrueba, pidPrueba, marcoDeUnaPaginaYUnID);

	pid = 6;
	size = 2;
	mode = DESTRUCTION;
	r = playingGod(pid, mode, size);

	 pidPrueba = 6;
	 numeroPaginaPrueba = 4;
	 marcoDeUnaPaginaYUnID = getFrame(pidPrueba,numeroPaginaPrueba);

	printf("\nEl marco de la pagina consultada (%d) del proceso (%d) es: %d\n", numeroPaginaPrueba, pidPrueba, marcoDeUnaPaginaYUnID);

	pid = 6;
	size = 32;
	r = resize(pid, size);

	pidPrueba = 6;
	numeroPaginaPrueba = 4;
	marcoDeUnaPaginaYUnID = getFrame(pidPrueba,numeroPaginaPrueba);

	printf("\nEl marco de la pagina consultada (%d) del proceso (%d) es: %d\n", numeroPaginaPrueba, pidPrueba, marcoDeUnaPaginaYUnID);

	//dumpPaginacion();

	//char* rIns = getInstructions(7);
	//int ultima;
	//char* rIns = getIndividualInstruction(7,3,&ultima); //0 es numero de instruccion
	//printf("rIns: %s\n", rIns);
	//printf("rIns ult: %d",ultima);
	f_address* physicalAddress = malloc(sizeof(f_address));
	physicalAddress->frame = 0;
	physicalAddress->offset = 0;
	accessUserSpace(6, READIT, physicalAddress, 32);
	accessUserSpace(6, WRITEIT, physicalAddress, 32);
	accessUserSpace(6, READIT, physicalAddress, 64);

    //Pruebas de TLB
    int pid = 6;
	int size = 3; 
	int mode = CREATION;
	int r = playingGod(pid, mode, size);

	pid = 7;
	size = 3; 
	mode = CREATION;
	r = playingGod(pid, mode, size);

	int testPid = 6;
	int testPage = 2;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 6;
	testPage = 0;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 6;
	testPage = 1;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 7;
	testPage = 1;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);

    int pid = 6;
	int size = 33; 
	resize(pid,size);
	dumpPaginacion();

	f_address* physicalAddress = malloc(sizeof(f_address));
	physicalAddress->frame = 1;
	physicalAddress->offset = 0;
	accessUserSpace(6, READIT, physicalAddress, 2);
	accessUserSpace(6, WRITEIT, physicalAddress, 2);
	accessUserSpace(6, READIT, physicalAddress, 2);

	/*size = 64; 
	resize(pid,size);
	dumpPaginacion();

	size = 96; 
	resize(pid,size);
	dumpPaginacion();

	pid = 15;
	size = 96; 
	resize(pid,size);
	dumpPaginacion();

	pid = 6;
	size = 1; 
	resize(pid,size);
	dumpPaginacion();

	pid = 15;
	size = 1; 
	resize(pid,size);
	dumpPaginacion();

	pid = 7;
	size = 1; 
	resize(pid,size);
	dumpPaginacion();

	pid = 7;
	size = 128; 
	resize(pid,size);
	dumpPaginacion();

	pid = 3;
	size = 64; 
	resize(pid,size);
	dumpPaginacion();
	//Pruebas de TLB
    /*int pid = 6;
	int size = 3; 
	int mode = CREATION;
	int r = playingGod(pid, mode, size);

	pid = 7;
	size = 3; 
	mode = CREATION;
	r = playingGod(pid, mode, size);

	int testPid = 6;
	int testPage = 2;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 6;
	testPage = 0;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 6;
	testPage = 1;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 7;
	testPage = 1;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);

	testPid = 6;
	testPage = 0;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);

	testPid = 7;
	testPage = 2;
	r = hitOrMiss(testPid,testPage);
	r = hitOrMiss(testPid,testPage);
	sleep(1);
    
    --

    int pid=1;
	int	size = 64; 
	resize(pid,size);
	dumpPaginacion();
	//0 * 32 + offset
	void* r = readInPagingMemory(pid, 4,1);
	//printf("Datos leidos: %s\n",r);
	writeInPagingMemory("hola",pid,strlen("hola"),5);
	
	r = readInPagingMemory(pid, 9,0);
	//printf("Datos leidos: %s\n",r);
	//printf("Main Memory: %p", mainMemory);

    int pid=1;
	int	size = 65; 
	resize(pid,size);
	dumpPaginacion();
	//0 * 32 + offset
	void* r = readInPagingMemory(pid, 4,33);
	printf("Datos leidos a: %s\n",r);

	writeInPagingMemory("hola",pid,strlen("hola"),35);
	r = readInPagingMemory(pid, 9,32);
	printf("Datos leidos b: %s\n",r);

	writeInPagingMemory("chau",pid,strlen("hola"),35);
	r = readInPagingMemory(pid, 9,32);
	printf("Datos leidos c: %s\n",r);

	writeInPagingMemory("hola",pid,strlen("hola"),5);
	r = readInPagingMemory(pid, 9,0);
	printf("Datos leidos d: %s\n",r);

	

	pid=2;
	size = 33; 
	resize(pid,size);
	dumpPaginacion();
	writeInPagingMemory("what",pid,strlen("hola"),5);
	r = readInPagingMemory(pid, 9,0);
	printf("Datos leidos d: %s\n",r);

	pid=1;
	size = 65; 
	resize(pid,size);
	dumpPaginacion();

	writeInPagingMemory("hey",pid,strlen("hey"),70);
	r = readInPagingMemory(pid, 9,70);
	printf("Datos leidos e: %s\n",r);
    
    */
