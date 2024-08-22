#include "client_k.h"

t_package* package(int connection)
{
	// Ahora toca lo divertido!
	char* readed;
	int lengthReaded;
	t_package* package = createPackage();
	printf("Escribi tu mensaje: ");
	readed = readline("> ");
	// Leemos y esta vez agregamos las lineas al paquete
	while((*readed) != '\0')
	{
		lengthReaded = strlen(readed) + 1;
		addPackage(package, readed, lengthReaded);
		printf("Escribi tu mensaje: ");
		readed = readline("> ");
	}
	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	free(readed);

	return package;

}

// void sendPcbToCpu(pcb* pcb) {
	
// }