#include "commons/bitarray.h"
#include "commons/temporal.h"
#include "utils_memory.h"

extern t_log* log_memory;
extern char* logMemoryString;

typedef enum {
	DESTRUCTION,
	CREATION
} godMode;

typedef enum {
	READIT,
	WRITEIT
} userAccessMode;

typedef struct
{
    int id;

}t_frame;

typedef struct
{
    int frame;
    int offset;
}f_address;

typedef struct
{
    int page;
    int offset;
}l_address;

//traduccion la hace el MMU accediendo a la TP consiguiendo el frame y armar la direccion fisica
//DL = x bits | y bits maxima cantidad de paginas por proceso 2^x y tamaño de pagina 2^y
// 2 | A50 = 10832 y 
//#pagina * tamaño pagina )+ desplazamiento = DL
//#frame * tamaño frame ) + desplazamiento = DF
//segunda pagina del proceso arranca en el byte 4096 (tam de pag)


typedef struct
{
    int id;
    int frame_ppal;
    int presencia;
    int uso;
    int modificado;
    int lock;
    int tiempo_uso;
    int tamanioDisponible;
    int fragInterna;
}t_page;

typedef struct
{
    int pid;
    int instructionSize;
    int pcbAddr;
    int lastUsedPageID;
    t_list* pages;
}t_pageTable;

//prototipos

int initPagination();
int pageAvailability();
char* asignarMemoriaBits(int);
void deleteAlgo(void*);
t_page* pageCreate(t_frame* , int , int);
t_list* findFreeFrames();
t_list* saveInPage(void* , int ,int);
void saveInPagingMemory(void* , int ,t_frame* ,int);
void ocupyFrame(t_frame* );
void ocupyPagingMemory(void* , int , int , int );
int obtenerTiempo();
int generarId();
void deleteList(t_list* );
char* callMeByYourName();
int findPID(t_page* );
t_list* findAllPages();
t_page* findAFramesPage(int );

//Funciones para visualizar

void showFrame(int);
void showPageFrames();
void dumpPaginacion();

t_pageTable* createPageTable(int , int );
t_list* pagesOccupiedByInstructions(char* , int);
int calculateLogicalAddress(t_page* , int );
void addPagesToPCB(t_pageTable* , t_list* );
void addPagesToTable(t_pageTable* , t_list* );
t_list* findAvailablePages(t_pageTable* );
t_pageTable* getPageTable(int );
int isThereRoomInYourPage(int );
void savePCBPagination(pcb* );
t_page* saveSomething(void* , int , t_pageTable* , int* );
t_list* fillPageSpace(void* , t_page* , int , int* , int);
int playingGod(int , godMode ,int );
void freeAFrame(int );
void deletePageTable(t_pageTable* );
void freePagingMemory();
void freeAPage(t_page* , t_pageTable* );
void freePages(t_list* , int , t_pageTable* , int , int );
void killAPage(t_page* );
int pagesHaveRoom(t_pageTable* );

void freePagesFromTheEnd(t_pageTable* , int );

int getFrame(int , int );

int writeInPagingMemory(char*, int , int , int);
char* readInPagingMemory(int , int , int );

int resize(int , int );

char* getInstructions(int);
char* getInstructionsFromPages(t_list* , t_pageTable* );
t_list* getPages(int , int , t_pageTable*, int );
void* copyPagesMP(t_list* );
void copyPages(t_list* , void* );
void copyPageAfter(t_page* , void* , int* );
char* getInstructionsPages(void* , int  );
char* getIndividualInstruction(int , int , int* );
char* separateInstructions(char* , int , int* );
void ocupyPhysicalAddress(void* algo, int size, int address, int pid);

void freeStringArray(char** );
int sizeStringArray(char** );
char* leer_a_partir_de_direccion(int pid, int opSize, int physicalAddress);
t_page *obtener_pagsig_de_dirfisica(uint32_t direccion_fisica, uint32_t pid);
uint32_t recalcular_direccion_fisica(t_page *pagina);
extern pthread_mutex_t mutexMemoria;
extern pthread_mutex_t mutexIdGlobal;

extern int memLengthInt;
extern int pageLengthInt;

extern int cantFrames;
extern void* mainMemory;
extern char* dataForBitArray;
extern t_bitarray* bitArrayOcupiedFrames;
extern t_list* pageTable;

extern int idGlobal;


