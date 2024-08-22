#ifndef INTERACTIVE_CONSOLE_H
#define INTERACTIVE_CONSOLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <commons/collections/queue.h>
#include "pcb.h"
#include "utils.h"

void interpetAndExecCommand(t_queue* wordList, char* absolutePath);
void printParameterScopeError();
char* toUpperCase(char* str); // Devuelve otro char con todas las letras em mayus, y libera de la memoria el char que pasamos por parametro
int endsWithSlash(char *str);
void removeLeadingSlash(char** str);

extern void changeMultiprogramming(char* value);
extern void runChangeMultiprogramming(char* value);

#endif /* INTERACTIVE_CONSOLE_H */
