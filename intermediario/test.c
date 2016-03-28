#include "comun.h"

int alta_usuario(SOCKADDR_IN *cli_addr, int port);
int baja_usuario(SOCKADDR_IN *cli_addr, int port);
int susc_usuario_tema(SOCKADDR_IN *cli_addr, const char *tema, int port);
int desusc_usuario_tema(SOCKADDR_IN *cli_addr, const char *tema, int port);
int notificar_tema_nuevo(const char *tema);
int notificar_tema_elim(const char *tema);
int notificar_nuevo_evento(const char *tema, const char *valor);
int respuesta(int tipo, int sckt);
int remove_tema(const char *tema);
int buscar_tema(const char *tema);
int remove_usuario_tema(const SOCKADDR_IN *cli_addr, const int index);
int buscar_usuario_tema(const SOCKADDR_IN *cli_addr, const int index);
int remove_usuario(SOCKADDR_IN *cli_addr);
int buscar_usuario(SOCKADDR_IN *cli_addr);
int send_message(TOPIC_MSG *msg, SOCKADDR_IN *cliente);

typedef struct topic
{
  char tp_nam[64];
  int mem_sz;
  SOCKADDR_IN *mem;
} TOPIC;

/* Para no tener que buscar tema por tema a la hora de notificar
   a todos los clientes */
typedef struct suscr
{
  SOCKADDR_IN susc_info;
}SUSCR;

int n_topics = 0, n_suscr = 0;
TOPIC *topics;
SUSCR *suscr;

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
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i;
  /* Obtenemos los temas del fichero y inicializamos los struct*/
  while((read = getline(&line, &len, file)) != -1)
    {
      strtok(line, "\n");
      sprintf(topics[n_topics].tp_nam, "%s", line);
      topics[n_topics].mem_sz = 0;
      topics[n_topics++].mem = malloc(sizeof(SOCKADDR_IN));
      topics = realloc(topics, (1+n_topics)*sizeof(TOPIC));
    }
  fclose(file);
  
#ifdef DEBUG /* debug inicialización de temas */
  for(i=0;i<n_topics;i++)
    {
      printf("Topic name:%s , mem_size:%d\n",
	     topics[i].tp_nam, topics[i].mem_sz);
    }
#endif
#ifdef DEBUG
  return 0;
#endif
  
  SOCKADDR_IN serv_addr, cli_addr;
  int sckt_s, sckt, port;
  port = atoi(argv[1]);
  socklen_t addr_sz = sizeof(SOCKADDR_IN);
  
  if((sckt_s = abrir_puerto_escucha(port, &serv_addr))==-1)
    exit(EXIT_FAILURE);

  while(1)
    {
      if((sckt = accept(sckt_s, (SOCKADDR *)&cli_addr, &addr_sz))==-1)
	{
	  perror("Error");
	  exit(EXIT_FAILURE);
	}
      unsigned char buff[4096] = {0};
      ssize_t tam;

      if((tam=recv(sckt, buff, MAX_REC_SZ, 0))==-1)
	{
	  perror("Error");
	  exit(EXIT_FAILURE);
	}

      TOPIC_MSG *msg;
      msg = deserialize(buff, (size_t)tam);
      
      switch(msg->op)
	{
	case GENEV:
	  if(notificar_nuevo_evento(msg->tp_nam, msg->tp_val)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	case CREAT:
	  if(notificar_tema_nuevo(msg->tp_nam)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	case ELIMT:
	  if(notificar_tema_elim(msg->tp_nam)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	case NEWSC:
	  if(alta_usuario(&cli_addr, msg->port)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	case FINSC:
	  if(baja_usuario(&cli_addr, msg->port)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	case ALTAT:
	  if(susc_usuario_tema(&cli_addr, msg->tp_nam, msg->port)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	case BAJAT:
	  if(desusc_usuario_tema(&cli_addr, msg->tp_nam, msg->port)==-1)
	    respuesta(ERROR, sckt);
	  else
	    respuesta(OK, sckt);
	  break;
	}
      close(sckt);

    }
  
  for(i=0;i<n_topics;i++)
    {
      free(topics[i].mem);
    }
  free(topics);
  free(suscr);
  
  return EXIT_SUCCESS;
}


int send_message(TOPIC_MSG *msg, SOCKADDR_IN *cliente)
{
  int sckt;
  if((sckt=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
    {
      perror("Error");
      return -1;
    }

  if(connect(sckt, (SOCKADDR*)cliente, sizeof(SOCKADDR_IN))==-1)
    {
      perror("Error");
      return -1;
    }

  size_t msg_sz;
  unsigned char *buf = 0;
  msg_sz = serialize(msg, &buf);
      
  ssize_t tam;
  if((tam=send(sckt, buf, msg_sz, 0))==-1)
    {
      perror("Error");
      exit(EXIT_FAILURE);
    }

  close(sckt);
  return 0;
}

int notificar_tema_nuevo(const char *tema)
{
  /* Si ya existe error */
  if(buscar_tema(tema)!=-1)
    return -1;

  /* Lo añadimos a la lista de temas */
  sprintf(topics[n_topics].tp_nam, "%s", tema);
  topics[n_topics].mem_sz = 0;
  topics[n_topics++].mem = malloc(sizeof(SOCKADDR_IN));
  topics = realloc(topics, (1+n_topics)*sizeof(TOPIC));

  /* Y notificamos a los suscriptores */
  TOPIC_MSG msg;
  bzero((char*)&msg, sizeof(TOPIC_MSG));
  msg.op = NUEVT;
  sprintf(msg.tp_nam, "%s", tema);

  int i;
  for(i=0; i<n_suscr; i++)
    {
      send_message(&msg, &(suscr[i].susc_info));
    }
  return 0;
}


int buscar_tema(const char *tema)
{
  int i, index;
  bool enc = false;
  for (i=0; i<n_topics && !enc; i++)
    {
      if(strcmp(topics[i].tp_nam, tema)==0)
	{
	  enc = true;
	  index = i;
	}
      
    }
  return enc? index : -1;
}

int remove_tema(const char *tema)
{
  int elem;
  if((elem=buscar_tema(tema))==-1)
    return -1;
  
  TOPIC *temp = malloc((n_topics-1) * sizeof(TOPIC));
  
  // copiar la parte anterior al tema
  memmove(temp, topics, (elem+1)*sizeof(TOPIC)); 

  // copiar la parte posterior al tema
  memmove(temp+elem, (topics)+(elem+1), (n_topics-elem)*sizeof(TOPIC));

  free(topics[elem].mem);
  free(topics);
  n_topics--;
  topics = temp;
  return 0;
}

