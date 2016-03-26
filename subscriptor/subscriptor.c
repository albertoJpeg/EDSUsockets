#include "comun.h"
#include "subscriptor.h"
#include "edsu_comun.h"
#include <pthread.h>

#define PUERTO_LIBRE 0

pthread_t notif;
int exit_status;

int activar_notificacion(void);
void *recibir_notif(void *sckt);
void (*evento)(const char*, const char*);
void (*atema)(const char*);
void (*btema)(const char*);

int alta_subscripcion_tema(const char *tema) {
  return enviar_mensaje(ALTAT, tema);
}

int baja_subscripcion_tema(const char *tema) {
  return enviar_mensaje(ALTAT, tema);
}

int inicio_subscriptor(void (*notif_evento)(const char *, const char *),
		       void (*alta_tema)(const char *),
		       void (*baja_tema)(const char *))
{
  evento = notif_evento;
  atema = alta_tema;
  btema = baja_tema;
  
  if(enviar_mensaje(NEWSC)==-1)
    return -1;
  
  if (activar_notificacion() == -1)
    {
      fprintf(stderr, "Error al activar las notificaciones\n");
      return -1;
    }
    
  return 0;
}
/* Avanzada */
int fin_subscriptor() {

  if(enviar_mensaje(FINSC)==-1)
    return -1;

  int *status;
  if(pthread_join(notif, (void**)&status)!=0)
    return -1;

  return *status==OK? 0: -1;
}

int activar_notificacion(void)
{
  SOCKADDR_IN cli_addr;
  int sckt, port = PUERTO_LIBRE;

  if((sckt = abrir_puerto_escucha(port, &cli_addr))==-1)
    return -1;
  
  
  if(pthread_create(&notif, NULL, recibir_notif, &sckt) != 0)
    {
      perror("Error");
      return -1;
    }

  return 0;
}

void *recibir_notif(void *sck)
{
  int sckt = *((int*)sck);
  socklen_t addr_sz = sizeof(SOCKADDR_IN);
  int sckt_n;
  SOCKADDR_IN serv_addr;
  int fin = 0;
  while(!fin)
    {
      if((sckt_n = accept(sckt, (SOCKADDR*)&serv_addr, &addr_sz))==-1)
	{
	  perror("Error");
	  exit_status = -1;
	  return(&exit_status);
	}

      unsigned char buff[4096] = {0};
      ssize_t tam;
      
      if((tam=recv(sckt, buff, MAX_REC_SZ, 0))==-1)
	{
	  perror("Error");
	  exit_status = -1;
	  return(&exit_status);
	}

      TOPIC_MSG *msgI;
      msgI = deserialize(buff, (size_t)tam);
  
      switch(msgI->op)
	{
	case NOTIF:
	  (*evento)(msgI->tp_nam, msgI->tp_val);
	  break;
	case NUEVT:
	  (*atema)(msgI->tp_nam);
	  break;
	case TEMAE:
	  (*btema)(msgI->tp_nam);
	  break;
	case OK:
	  fin = OK;
	  break;
	case ERROR:
	  fin = ERROR;
	  break;
	}
      
      close(sckt_n);
      exit_status = fin;
      return(&exit_status);
    }
  return NULL;
}  
