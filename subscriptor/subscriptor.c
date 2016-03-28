#include "comun.h"
#include "subscriptor.h"
#include "edsu_comun.h"
#include <pthread.h>

#define PUERTO_LIBRE 0

pthread_t thread_notif;
int exit_status;
int puerto_notif;
int sckt_notif;

int activar_notificacion(void);
void *recibir_notif(void *sckt);
void (*evento)(const char*, const char*);
void (*atema)(const char*);
void (*btema)(const char*);

int alta_subscripcion_tema(const char *tema) {
  return enviar_mensaje(ALTAT, tema, puerto_notif);
}

int baja_subscripcion_tema(const char *tema) {
  return enviar_mensaje(BAJAT, tema, puerto_notif);
}

int inicio_subscriptor(void (*notif_evento)(const char *, const char *),
		       void (*alta_tema)(const char *),
		       void (*baja_tema)(const char *))
{
  evento = notif_evento;
  atema = alta_tema;
  btema = baja_tema;
  
  SOCKADDR_IN cli_addr;
  int port = PUERTO_LIBRE;
  
  if((sckt_notif = abrir_puerto_escucha(port, &cli_addr))==-1)
    return -1;

  puerto_notif = ntohs(cli_addr.sin_port);

  if(enviar_mensaje(NEWSC, puerto_notif)==-1)
    return -1;

/* #ifdef DEBUG */
/*   if(enviar_mensaje(ALTAT, "prueba", puerto_notif)==-1) */
/*     return -1; */
/* #endif */
  
  if(pthread_create(&thread_notif, NULL, recibir_notif, NULL) != 0)
    {
      perror("Error");
      return -1;
    }

/* #ifdef DEBUG */
/*   sleep(30); */
/*   fin_subscriptor(); */
/* #endif */
  
  return 0;
}
/* Avanzada */
int fin_subscriptor() {

  if(enviar_mensaje(FINSC, puerto_notif)==-1)
    return -1;
  
/* #ifdef DEBUG */
/*   pthread_cancel(thread_notif); */
  
/*   if(pthread_join(thread_notif, NULL)!=0) */
/*     return -1; */
/* #endif */
  
  close(sckt_notif);
  sckt_notif = 0;
  puerto_notif = 0;

  return 0;
}

void *recibir_notif(void *p)
{
  int sckt_n;
  socklen_t addr_sz = sizeof(SOCKADDR_IN);
  SOCKADDR_IN serv_addr;
  
  while(1)
    {
      if((sckt_n = accept(sckt_notif, (SOCKADDR*)&serv_addr, &addr_sz))==-1)
	{
	  perror("Error");
	  exit_status = -1;
	  return &exit_status;
	}
      
      unsigned char buff[4096] = {0};
      ssize_t tam;
      
      if((tam=recv(sckt_n, buff, MAX_REC_SZ, 0))==-1)
	{
	  perror("Error");
	  exit_status = -1;
	  return &exit_status;
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
	}    
      close(sckt_n);      
    }
  
  return(&exit_status);
}  
