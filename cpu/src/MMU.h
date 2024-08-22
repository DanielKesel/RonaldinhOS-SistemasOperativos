#include <math.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include "utils.h"
#include "utils_cpu.h"

extern t_log* log_cpu;
extern char* logCpuString;

uint32_t logicalToPhysicalAddress(uint32_t logicalAddress, uint32_t pid);
extern int32_t pagesLength;
extern char* CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;
extern t_config *config_CPU;
extern int memoryConnection;




uint32_t hitOrMiss(uint32_t pid, int page);
extern t_list* TLB;

int isThereRoomInTLB();
int insertTLBRow(int, int, int);

int insertScheduledTLBRow(int , int , int , char* );
uint32_t buscarFrameEnMemoria(uint32_t pid, int page);


TLB_ROW* findMinimum(t_link_element* );

long get_elapsed_time_micro();