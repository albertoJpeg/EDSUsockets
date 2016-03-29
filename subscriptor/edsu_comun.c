/*
  Incluya en este fichero todas las implementaciones que pueden
  necesitar compartir los módulos editor y subscriptor,
  si es que las hubiera.
*/
#include "comun.h"
#include "edsu_comun.h"

int empaquetar_y_enviar(TOPIC_MSG *msg, int tipo, int sckt);
int conectar(ENV *ent);
int obtener_entorno(ENV *ent);

//tipo, tema, valor
int enviar_mensaje(int tipo, ...)
{
  char *tema;
  char *valor;
  int puerto;
  va_list argv;

  switch(tipo)
    {
    case GENEV://tema, valor
      va_start(argv, tipo);
      tema = va_arg(argv, char *);
      valor = va_arg(argv, char *);
      break;
    case NEWSC://no se recibe nada
    case FINSC:
      va_start(argv, tipo);
      puerto = va_arg(argv, int);
      break;
    case ALTAT:
    case BAJAT:
      va_start(argv, tipo);
      tema = va_arg(argv, char *);
      puerto = va_arg(argv, int);
      break;
    default:
      va_start(argv, tipo);
      tema = va_arg(argv, char *);
    }
  
  ENV ent;
  if(obtener_entorno(&ent)==-1)
    return -1;
    
  int sckt;
  if((sckt = conectar(&ent))==-1)
    return -1;
  
  TOPIC_MSG msg;
  bzero((char*)&msg, sizeof(TOPIC_MSG));

  switch(tipo)
    {
    case GENEV:
      /* Enviamos mensaje de creación de tema */
      msg.op = tipo;
      sprintf(msg.tp_nam, "%s", tema);
      sprintf(msg.tp_val, "%s", valor);

      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;
    case CREAT:
      /* Enviamos mensaje de creación de tema */
      msg.op = tipo;
      sprintf(msg.tp_nam, "%s", tema);

      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;
    case ELIMT:
      /* Enviamos mensaje de borrado de tema */
      msg.op = tipo;
      sprintf(msg.tp_nam, "%s", tema);

      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;
    case NEWSC:
      /* Enviamos mensaje de nueva suscripción */
      msg.op = tipo;
      msg.port = puerto;

      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;
    case FINSC:
      /* Enviamos mensaje de fin suscripción */
      msg.op = tipo;
      msg.port = puerto;
      
      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;  
    case ALTAT:
      /* Enviamos mensaje de alta a tema */
      msg.op = tipo;
      msg.port = puerto;
      sprintf(msg.tp_nam, "%s", tema);
      
      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;
    case BAJAT:
      /* Enviamos mensaje de baja a tema */
      msg.op = tipo;
      msg.port = puerto;
      sprintf(msg.tp_nam, "%s", tema);

      if(empaquetar_y_enviar(&msg, tipo, sckt)==-1)
	return -1;
      
      close(sckt);
      break;
    }
  
  va_end(argv);
  return 0;

}

int empaquetar_y_enviar(TOPIC_MSG *msg, int tipo, int sckt)
{
  size_t msg_sz;
  unsigned char *buf = 0;
  msg_sz = serialize(msg, &buf);
      
  ssize_t tam;
  if((tam=send(sckt, buf, msg_sz, 0))==-1)
    {
      perror("Error");
      return -1;
    }
      
  unsigned char buff[4096] = {0};
      
  if((tam=recv(sckt, buff, MAX_REC_SZ, 0))==-1)
    {
      perror("Error");
      return -1;
    }
      
  TOPIC_MSG *msgI;
  msgI = deserialize(buff, (size_t)tam);
      
  if(msgI->op==ERROR)
    {
      switch(tipo)
	{
	case GENEV:
	  fprintf(stderr, "EDITOR: Error al generar evento\n");
	  break;
	case CREAT:
	  fprintf(stderr, "EDITOR: Error al crear tema\n");
	  break;
	case ELIMT:
	  fprintf(stderr, "EDITOR: Error al eliminar tema\n");
	  break;
	case NEWSC:
	  fprintf(stderr, "SUSC: Error al suscribirse al servicio\n");
	  break;
	case FINSC:
	  fprintf(stderr, "SUSC: Error al finalizar suscripción al servicio\n");
	  break;
	case ALTAT:
	  fprintf(stderr, "SUSC: Error en pedir alta a tema\n");
	  break;
	case BAJAT:
	  fprintf(stderr, "SUSC: Error en pedir baja de tema\n");
	  break;
	case NUEVT: /* Etiqueta interm <-> suscrip, para pedir avisos en nuevos temas, o para enviar la notificación en si */
	  fprintf(stderr, "SUSC: Error en pedir notificaciones de nuevos temas\n");
	  break;
	case TEMAE: /* Mismo caso que NUEVT */
	  fprintf(stderr, "SUSC: Error en pedir notificaciones de temas eliminados\n");
	  break;

	}
      return -1;
    }
  free(buf);//serialize
  free(msgI);//deserialize
  return 0;
}

int obtener_entorno(ENV *ent)
{
  if((ent->SERVIDOR = getenv("SERVIDOR"))==NULL)
    {
      fprintf(stderr,"Error: Variable de entorno SERVIDOR no definida\n");
      return -1;
    }


  if((ent->PUERTO = getenv("PUERTO"))==NULL)
    {
      fprintf(stderr,"Error: Variable de entorno PUERTO no definida\n");
      return -1;
    }
  return 0;
}

int conectar(ENV *ent)
{
  int sckt, port;
  port = atoi(ent->PUERTO);
  struct hostent *netdb;
  SOCKADDR_IN serv_addr;
  size_t addr_sz_s = sizeof(SOCKADDR_IN);
  socklen_t addr_sz_sc = sizeof(SOCKADDR_IN);

  netdb = gethostbyname(ent->SERVIDOR);

  bzero((char*)&serv_addr, addr_sz_s);

  serv_addr.sin_family = AF_INET;
  memcpy(&(serv_addr.sin_addr), netdb->h_addr, netdb->h_length);
  serv_addr.sin_port = htons(port);

  if((sckt=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
    {
      perror("Error");
      return -1;
    }

  if(connect(sckt, (SOCKADDR*)&serv_addr, addr_sz_sc)==-1)
    {
      perror("Error");
      return -1;
    }
  
  return sckt;
}
