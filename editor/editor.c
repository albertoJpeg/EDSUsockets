#include "comun.h"
#include "editor.h"
#include "edsu_comun.h"

int generar_evento(const char *tema, const char *valor)
{
  return enviar_mensaje(GENEV, tema, valor);
}

/* solo para la version avanzada */
int crear_tema(const char *tema)
{
  return enviar_mensaje(CREAT, tema);
}

/* solo para la version avanzada */
int eliminar_tema(const char *tema)
{
  return enviar_mensaje(ELIMT, tema);
}

