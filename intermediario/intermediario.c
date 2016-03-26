#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "comun.h"

int alta_usuario(SOCKADDR *cli_addr);
int baja_usuario(SOCKADDR *cli_addr);
int susc_usuario_tema(SOCKADDR *cli_addr, const char *tema);
int desusc_usuario_tema(SOCKADDR *cli_addr, const char *tema);
int notificar_tema_nuevo(const char *tema);
int notificar_tema_elim(const char *tema);
int notificar_nuevo_evento(const char *tema, const char *valor);

typedef struct topic
{
  char *tp_nam;
  int mem_sz;
  SOCKADDR *mem;
} TOPIC;

/* Para no tener que buscar tema por tema a la hora de notificar
a todos los clientes */
typedef struct suscr
{
  SOCKADDR susc_info;
}SUSCR;

/* Clientes que quieren que se les avise por cada nuevo/elim tema*/
typedef struct suscr_ab
{
  SOCKADDR susc_info;
}SUSCR_AB;

int n_topics = 0, n_suscr = 0, n_avisos = 0;
TOPIC *topics;
SUSCR *suscr;
SUSCR_AB *avisos;


int main(int argc, char *argv[])
{
  
  if (argc!=3)
    {
      fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
      exit(EXIT_FAILURE);
    }

  FILE *file;
  if((file = fopen(argv[2], "r")) == NULL)
    {
      perror("Error");
      exit(EXIT_FAILURE);
    }
  
  topics = malloc(sizeof(TOPIC));
  suscr = malloc(sizeof(SUSCR));
  avisos = malloc(sizeof(SUSC_AB));
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i;
  /* Obtenemos los temas del fichero y inicializamos los struct*/
  while((read = getline(&line, &len, file)) != -1)
    {
      strtok(line, "\n");
      topics[n_topics].tp_nam = strdup(line);
      topics[n_topics].mem_sz = 0;
      topics[n_topics++].mem = malloc(sizeof(SOCKADDR));
      topics = realloc(topics, (1+n_topics)*sizeof(TOPIC));
    }
  
#ifdef DEBUG /* debug inicialización de temas */
  for(i=0;i<n_topics;i++)
    {
      printf("Topic name:%s , mem_size:%d\n",
	     topics[i].tp_nam, topics[i].mem_sz);
    }
#endif
  
  SOCKADDR_IN serv_addr;
  SOCKADDR cli_addr;
  int sckt_s, sckt_c, port;
  port = atoi(argv[1]);
  socklen_t addr_sz = sizeof(SOCKADDR_IN);
  
  if((sckt_s = abrir_puerto_escucha(port, &serv_addr))==-1)
    exit(EXIT_FAILURE);

  while(1)
    {
      if((sckt_c = accept(sckt_s, &cli_addr, &addr_sz))==-1)
	{
	  perror("Error");
	  exit(EXIT_FAILURE);
	}
      unsigned char buff[4096] = {0};
      unsigned char resp[MAX_REC_SZ];
      int offset = 0;
      
      while((tam=recv(sckt, (void*)resp, MAX_REC_SZ, 0))>0)
	{
	  memcpy(buff + offset, &buf, (size_t)tam);
	  offset+=tam;
	}

      TOPIC_MSG *msg;
      msg = deserialize(buff, (size_t)offset);
  
      switch(msg->op)
	{
	case GENEV:
	  notificar_nuevo_evento(msg->tp_nam, msg->tp_val)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);
	  break;
	case CREAT:
	  notificar_tema_nuevo(msg->tp_nam)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);
	  break;
	case ELIMT:
	  notificar_tema_elim(msg->tp_nam)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);
	  break;
	case NEWSC:
	  alta_usuario(&cli_addr)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);
	  break;
	case FINSC:
	  baja_usuario(&cli_addr)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);
	  break;
	case ALTAT:
	  susc_usuario_tema(&cli_addr, msg->tp_nam)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);
	  break;
	case BAJAT:
	  desusc_usuario_tema(&cli_addr, msg->tp_nam)==-1? respuesta(ERROR, sckt) : respuesta(OK, sckt);	 
	  break;
	}
      close(sckt_c);

    }
  
  for(i=0;i<n_topics;i++)
    {
      free(topics[n_topics].tp_nam);
      free(topics[n_topics].mem);
    }
  free(topics);
  fclose(file);
  return EXIT_SUCCESS;

}

int enviar_mensaje(TOPIC_MSG *msg, SOCKADDR *cliente)
{
  int sckt;
  if((sckt=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
    {
      perror("Error");
      return -1;
    }

  if(connect(sckt, &cliente, sizeof(SOCKADDR))==-1)
    {
      perror("Error");
      return -1;
    }

  size_t msg_sz;
  unsigned char *buf = 0;
  msg_sz = serialize(msg, &buf);
      
  ssize_t tam;
  while((tam=send(sckt, buf, msg_sz, 0))>0);

  close(sckt);
  return 0;
}

int notificar_nuevo_evento(const char *tema, const char *valor)
{
  int event;
  if((event=buscar_tema(tema))==-1)
    return -1;
  
  SOCKADDR cliente;
  TOPIC_MSG msg;
  bzero((char*)&msg, sizeof(TOPIC_MSG));
  msg.op = NOTIF;
  sprintf(msg.tp_nam, "%s", tema);
  sprintf(msg.tp_val, "%s", valor);
  
  int i;
  for(i=0; i<topics[event].mem_sz; i++)
    {
      enviar_mensaje(&msg, &topics[event].mem[i]);
    }
  return 0;
}

int notificar_tema_nuevo(const char *tema)
{
  topics[n_topics].tp_nam = strdup(tema);
  topics[n_topics].mem_sz = 0;
  topics[n_topics++].mem = malloc(sizeof(SOCKADDR));
  topics = realloc(topics, (1+n_topics)*sizeof(TOPIC));

  SOCKADDR cliente;
  TOPIC_MSG msg;
  bzero((char*)&msg, sizeof(TOPIC_MSG));
  msg.op = NUEVT;
  sprintf(msg.tp_nam, "%s", tema);

  int i;
  for(i=0; i<n_avisos; i++)
    {
      enviar_mensaje(&msg, &avisos[i].susc_info);
    }
  return 0;
}

int notificar_tema_elim(const char *tema)
{
  if(remove_tema(tema)==-1)
    return -1;
  
  SOCKADDR cliente;
  TOPIC_MSG msg;
  bzero((char*)&msg, sizeof(TOPIC_MSG));
  msg.op = ELIMT;
  sprintf(msg.tp_nam, "%s", tema);

  int i;
  for(i=0; i<n_avisos; i++)
    {
      enviar_mensaje(&msg, &avisos[i].susc_info);
    }
  return 0;

}

int alta_usuario(SOCKADDR *cli_addr)
{
  
}

int baja_usuario(SOCKADDR *cli_addr)
{
  
}

int susc_usuario_tema(SOCKADDR *cli_addr, const char *tema)
{
  
}

int desusc_usuario_tema(SOCKADDR *cli_addr, const char *tema)
{
  
}

int buscar_tema(const char *tema)
{
  int i, enc = 0;
  for (i=0; i<n_topics, !enc; i++)
    {
      if(strcmp(topics[i].tp_nam, tema)==0)
	enc = i;
    }
  return enc? enc : -1;
}

int remove_tema(const char *tema)
{
  int elem;
  if((elem=buscar_tema(tema))==-1)
    return -1;
  
    TOPIC *temp = malloc((n_topics-1) * sizeof(TOPIC));

    // copiar la parte anterior al tema
    memmove(temp, *topics, (elem+1)*sizeof(TOPIC)); 

    // copiar la parte posterior al tema
    memmove(temp+elem, (*topics)+(elem+1), (n_topics-elem)*sizeof(TOPIC));

    n_topics--;
    free(*topics);
    *topics = temp;
    return 0;
}

int remove_cliente
