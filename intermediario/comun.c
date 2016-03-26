/*
  Incluya en este fichero todas las implementaciones que pueden
  necesitar compartir todos los módulos (editor, subscriptor y
  proceso intermediario), si es que las hubiera.
*/

#include "comun.h"

size_t serialize(const TOPIC_MSG* topic, unsigned char** buf)
{
  
  const size_t tp_nam_sz, tp_val_sz, topic_size;  
  tp_nam_sz = topic->tp_nam ? strlen(topic->tp_nam) : 0;
  tp_val_sz   = topic->tp_val ? strlen(topic->tp_val) : 0;
  tp_sz = sizeof(topic->op) + sizeof(size_t) + tp_nam_sz + sizeof(size_t) + tp_val_sz;

  if((*buf = calloc(1, topic_size))==NULL)
    {
      perror("Error");
      return -1;
    }
  
  size_t offset = 0;
  unsigned int tmp;

  // op
  tmp = htonl(topic->op);
  memcpy(*buf + offset, &tmp, sizeof(tmp));
  offset += sizeof(tmp);
      
  // tamaño de tp_nam y tp_nam
  tmp = htonl(tp_nam_sz);
  memcpy(*buf + offset, &tmp, sizeof(tmp));
  offset += sizeof(tmp);      
  memcpy(*buf + offset, topic->tp_nam, tp_nam_sz);
  offset += tp_nam_sz;

  // tamaño de tp_val y tp_val
  tmp = htonl(tp_val_sz);
  memcpy(*buf + offset, &tmp, sizeof(tmp));
  offset += sizeof(tmp);
  memcpy(*buf + offset, topic->tp_val, tp_val_sz);
   
  return tp_sz;
}


TOPIC_MSG *deserialize(const unsigned char *buf, const size_t bufSz)
{
  /* 4 bytes de int, 4 por cada size_t y los string de tamaño 0*/
  static const size_t MIN_BUF_SZ = 12;

  TOPIC_MSG *topic = 0;

  if (buf && bufSz < MIN_BUF_SZ)
    {
      fprintf("El tamaño del buffer es menor que el mínimo\n");
      return -1;
    }
      
  if((topic = calloc(1, sizeof(TOPIC_MSG)))==NULL)
    {
      perror("Error");
      return -1;
    }
      
  size_t tmp = 0, offset = 0;

  // obtenemos op
  memcpy(&tmp, buf + offset, sizeof(tmp));
  tmp = ntohl(tmp);
  memcpy(&topic->op, &tmp, sizeof(topic->op));
  offset  += sizeof(topic->op);

  // obtenemos el tamaño de tp_nam
  memcpy(&tmp, buf + offset, sizeof(tmp));
  tmp = ntohl(tmp);
  offset  += sizeof(tmp);

  if (tmp > 0)
    {
      // obtenemos tp_nam
      //topic->tp_nam = calloc(1, tmp + 1);
      memcpy(topic->tp_nam, buf + offset, tmp);

      if(strlen(topic->tp_nam) != (size_t)tmp)
	{
	  fprintf("El tamaño del nombre del tema no coincide con el enviado\n");
	  return -1;
	}
      offset  += tmp;
    }

  // obtenemos el tamaño de tp_val
  memcpy(&tmp, buf + offset, sizeof(tmp));
  tmp = ntohl(tmp);
  offset  += sizeof(tmp);

  if (tmp > 0)
    {
      // obtenemos tp_val
      //topic->tp_val = calloc(1, tmp + 1);
      memcpy(topic->tp_val, buf + offset, tmp);

      if(strlen(topic->tp_val) != tmp)
	{
	  fprintf("El tamaño del valor del tema no coincide con el enviado\n");
	  return -1;
	}

    }
      
  return topic;
}

int abrir_puerto_escucha(int port, SOCKADDR_IN *serv_addr)
{
  int sckt;
  size_t addr_sz_s = sizeof(SOCKADDR_IN);
  socklen_t addr_sz_sc = sizeof(SOCKADDR_IN);

  if((sckt=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
    {
      perror("Error");
      return -1;
    }
  
  bzero((char*)serv_addr, addr_sz_s);
  
  serv_addr->sin_family=AF_INET;
  serv_addr->sin_addr.s_addr=inet_addr(INADDR_ANY);
  serv_addr->sin_port=htons(port);
    
  if(bind(sckt, (SOCKADDR*)serv_addr, addr_sz_sc)==-1)
    {
      perror("Error");
      close(sckt);
      return -1;
    }
  
  if(listen(sckt, MAX_CONNECT)==-1)
    {
      perror("Error");
      close(sckt);
      return -1;
    }
  
  if(getsockname(sckt, (SOCKADDR*)serv_addr, &addr_sz_sc)==-1)
    {
      perror("Error");
      close(sckt);
      return -1;
    }
  
  return sckt;
}
