#include "MMU.h"

uint32_t logicalToPhysicalAddress(uint32_t logicalAddress, uint32_t pid){
    
    if(pagesLength == -1) {
        log_error(log_cpu, "Error!, no hemos obtenido todavía el tamaño de la pagina por lo que no podemos hacer la traducción de DL a DF\n");
        return -1;
    }
	int nPage = floor(logicalAddress/ pagesLength); 
	int desplazamiento = logicalAddress - nPage * pagesLength;
	//buscar en TLB el marco/frame
    //sino buscar en memoria el marco/frame
    //aca hace las funciones hitOrMiss
    //se hardcodea el marco/frame con valor igual a nPagina.
    int frame = hitOrMiss(pid, nPage); 
    if(frame == -1){
        log_info(log_cpu, "NO SE ENCONTRO EN TLB. SE BUSCARA EN MEMORIA\n");
        //print_rectangle("BUSCANDO EN MEMORIA");
        frame = buscarFrameEnMemoria(pid, nPage);
    } else {
        //print_rectangle("ENCONTRADO EN TLB");
    }
    log_info(log_cpu, "PAGINA: %d", nPage);
    log_info(log_cpu, "FRAME: %d", frame);
    log_info(log_cpu, "DESPLAZAMIENTO: %d", desplazamiento);
    return (frame * pagesLength + desplazamiento);
}

uint32_t buscarFrameEnMemoria(uint32_t pid, int page) {
    
        uint32_t frame;
        solicitudFrameMemoria* futuroFrame = NUEVA_SOLICITUD_FRAME_MEMORIA(pid, page);
        send_SOLICITUD_FRAME_MEMORIA(futuroFrame, memoryConnection);
        free(futuroFrame); // VER SI SOLUCIONA MEMORY LEAK 216 BYTES
        recvInt(&frame, memoryConnection);

        logGetFrame((int)pid, page, (int)frame);

        log_info(log_cpu, "EL frame recibido es: %d\n", frame);
		//ahora que tengo el frame, registro una entrada a la TLB con ese pid, pagina y frame
		//para ello, primero debo verificar si hay lugar
        if(getValue(CANTIDAD_ENTRADAS_TLB) == 0) {return frame;}
		int tlbRoom = isThereRoomInTLB(TLB, CANTIDAD_ENTRADAS_TLB);
		if(tlbRoom > 0)
		{
			log_info(log_cpu, "Habia espacio asi que insertemos");
			insertTLBRow(pid, page, frame);
		}else
		{
			log_info(log_cpu, "No habia espacio asi que procedemos a usar algoritmo");
            insertScheduledTLBRow(pid, page, frame, ALGORITMO_TLB);
		}
        return frame;
}

//DE ACA PARA ABAJO ES DE TLB

uint32_t hitOrMiss(uint32_t pid, int page)
{
    //char* algoritmoReemplazo = "FIFO"; //esto viene de archivo de config
    //busco en la TLB para ver si lo puedo encontrar
    if (getValue(CANTIDAD_ENTRADAS_TLB) == 0) {return -1;};
    int hasTheRow(TLB_ROW* row){
        return (row->pid) == pid && (row->page)==page;
    }
    /*void showTLB(TLB_ROW* row)
    {
        printf("\npid: %d",row->pid);
        printf("\npage: %d",row->page);
        printf("\nframe: %d",row->frame);
        printf("\ntime: %d\n",row->lastUsedTime);
    }
    list_iterate(TLB, (void *)showTLB);*/

    if(list_find(TLB, (void*)hasTheRow) != NULL)
    {
        //printf("\nHit\n");
        
        logTlbHit((int)pid, page);

        /*TLB_ROW* row = malloc(sizeof(TLB_ROW));
        row->frame = frame;
        row->pid = pid;
        row->page = page;
        row->lastUsedTime = time(NULL);*/
        TLB_ROW* theRow = list_find(TLB, (void*)hasTheRow);
        theRow->lastUsedTime = get_elapsed_time_micro();
        return theRow->frame;
    } 
            
    logTlbMiss((int)pid, page);

    return -1;
}

int isThereRoomInTLB()
{
    int size = list_size(TLB);
    //printf("\nTLB size: %d\n", size);
    //printf("\nMax TLB size: %d", getValue(CANTIDAD_ENTRADAS_TLB));
    if(size < getValue(CANTIDAD_ENTRADAS_TLB))
        return 1;
    else 
        return -1;
}

int insertTLBRow(int pid, int page, int frame)
{
    TLB_ROW* row = malloc(sizeof(TLB_ROW));
    row->frame = frame;
    row->pid = pid;
    row->page = page;
    row->lastUsedTime = get_elapsed_time_micro();
    //print_rectangle("Se inserto en TLB");
    if(list_add(TLB, row)!=NULL)
        return 1;
    else 
        return -1;
}

int insertScheduledTLBRow(int pid, int page, int frame, char* algoritmoReemplazo)
{
    TLB_ROW* insertedRow = malloc(sizeof(TLB_ROW));
    insertedRow->frame = frame;
    insertedRow->pid = pid;
    insertedRow->page = page;
    insertedRow->lastUsedTime = get_elapsed_time_micro();
    if(strcmp(algoritmoReemplazo, "FIFO") == 0)
    {
        log_info(log_cpu, "Reemplazo FIFO");
        int i = 0;
        //el de arriba de todo va a ser quitado, moviendo todos los demas una posicion arriba e insertando el nuevo elemento al fondo
        void moveUp(TLB_ROW* row)
        {
            if(i != 0)
                list_replace(TLB, i-1, row); 
            i++;
        }

        list_iterate(TLB, (void *)moveUp);
        list_replace(TLB, getValue(CANTIDAD_ENTRADAS_TLB) -1,insertedRow);
        //s
        /*
        void showTLB(TLB_ROW* row)
        {
            printf("\npid: %d",row->pid);
            printf("\npage: %d",row->page);
            printf("\nframe: %d",row->frame);
            printf("\ntime: %d\n",row->lastUsedTime);
        }
        list_iterate(TLB, (void *)showTLB);
        */
 
    }else if(strcmp(algoritmoReemplazo, "LRU") == 0)
    {
        log_info(log_cpu, "Reemplazo LRU\n");
        TLB_ROW* minRow = findMinimum(TLB->head);
        void showTLB(TLB_ROW* row)
        {
            log_info(log_cpu, "\npid: %d",row->pid);
            log_info(log_cpu, "\npage: %d",row->page);
            log_info(log_cpu, "\nframe: %d",row->frame);
            log_info(log_cpu, "\ntime: %d\n",row->lastUsedTime);
        }
        list_iterate(TLB, (void *)showTLB);

        list_remove_element(TLB, minRow);
        list_add(TLB,insertedRow);
        log_info(log_cpu, "Se reemplaza la pagina %i", minRow->page);
    }
    return 1;
}

TLB_ROW* findMinimum(t_link_element* head) {

    TLB_ROW* row = head->data;
    int min = row->lastUsedTime;
    TLB_ROW* minRow = row;
    t_link_element* current = head->next;
    while (current != NULL) {
        row = current->data;
        if (row->lastUsedTime < min) {
            min = row->lastUsedTime;
            minRow = row;
        }
        current = current->next;
    }
    return minRow;
}

long get_elapsed_time_micro() {
    static struct timespec start;
    struct timespec now;
    
    // Inicializar start solo una vez
    if (start.tv_sec == 0 && start.tv_nsec == 0) {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    long elapsed = (now.tv_sec - start.tv_sec) * 1000000L + (now.tv_nsec - start.tv_nsec) / 1000L;
    return elapsed;
}