#include "subscriptor.h"
#include "comun.h"
#include "edsu_comun.h"
#include <pthread.h>

pthread_t notif;

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
  if(enviar_mensaje(NEWSC)==-1)
    return -1;
  
  if (activar_notificacion() == -1)
    {
      fprintf("Error ");
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

  if(*status == OK)
    {
      return 0;
    }
  else
    {
      return -1;
    }
}

int activar_notificacion(void)
{
  SOCKADDR_IN cli_addr;
  int sckt, port = PUERTO_LIBRE;

  if((sckt = abrir_puerto_escucha(port, &cli_addr))==-1)
    return -1;
  
  
  if(pthread_create(&notif, NULL, recibir_notif, (void*)sckt) != 0)
    {
      perror("Error");
      return -1;
    }

  return 0;
}

void *recibir_notif(void *sckt)
{
  socklen_t addr_sz = sizeof(SOCKADDR_IN);
  int sckt_n;
  SOCKADDR_IN serv_addr;

  int fin = 0;
  while(!fin)
    {
      if((sckt_n = accept(sckt, (SOCKADDR*)&serv_addr, &addr_sz))==-1)
	{
	  perror("Error");
	  pthread_exit(1);
	}

      unsigned char buff[4096] = {0};
      unsigned char resp[MAX_REC_SZ];
      int offset = 0;
      
      while((tam=recv(sckt, (void*)resp, MAX_REC_SZ, 0))>0)
	{
	  memcpy(buff + offset, &buf, (size_t)tam);
	  offset+=tam;
	}

      TOPIC_MSG *msgI;
      msgI = deserialize(buff, (size_t)offset);
  
      switch(msgI->op)
	{
	case NOTIF:
	  (*notif_evento)(msgI->tp_nam, msgI->tp_val);
	  break;
	case NUEVT:
	  (*alta_tema)(msgI->tp_nam);
	  break;
	case TEMAE:
	  (*baja_tema)(msgI->tp_nam);
	  break;
	case OK:
	  fin = OK;
	  break;
	case ERROR:
	  fin = ERROR;
	  break;
	}
      
      close(sckt_n);
      pthread_exit(fin);
    }
}  
