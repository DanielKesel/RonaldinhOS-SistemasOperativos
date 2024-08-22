#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "pcb.h"
#include "states.h"

#include "client.h"
#include "utils.h"

t_package* package(int);

//void sendPcbToCpu(pcb* pcb);