/*
   Incluya en este fichero todas las definiciones que pueden
   necesitar compartir los módulos editor y subscriptor,
   si es que las hubiera.
*/

#ifndef EDSU_H
#define EDSU_H

#include <stdarg.h>

typedef struct env
{
  char *SERVIDOR;
  char *PUERTO;
}ENV;

int enviar_mensaje(int tipo, ...);

#endif
