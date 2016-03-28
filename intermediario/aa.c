#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct sockaddr_in SOCKADDR_IN;

typedef struct suscr
{
  SOCKADDR_IN susc_info;
}SUSCR;

SUSCR *suscr;
int n_suscr;

int main()
{
  suscr = malloc(sizeof(SOCKADDR_IN));
  SOCKADDR_IN ss1;
  SOCKADDR_IN ss2;
  alta_usuario(&ss1, 201);
  alta_usuario(&ss2, 202);

  return 0;

}

int alta_usuario(SOCKADDR_IN *cli_addr, int port)
{
  cli_addr->sin_port = htons(port);
  
  /* Si ya existe usuario error */
  if(buscar_usuario(cli_addr)!=-1)
    return -1;

  suscr[n_suscr++].susc_info = *cli_addr;
  /* Lo añadimos a la lista de suscriptores */
  /* memmove(&(suscr[n_suscr].susc_info), cli_addr, sizeof(SOCKADDR_IN)); */
  suscr = realloc(suscr, (1+n_suscr)*sizeof(SUSCR));  
  return 0;
}

int buscar_usuario(SOCKADDR_IN *cli_addr)
{
  int i;

  for(i=0; i<n_suscr; i++)
    {
      if(suscr[i].susc_info.sin_addr.s_addr == cli_addr->sin_addr.s_addr &&
	 suscr[i].susc_info.sin_port == cli_addr->sin_port)
	return i;
    }
  return -1;
}


int remove_usuario(SOCKADDR_IN *cli_addr)
{
  int i;
  for(i=0; i<n_topics; i++)
    {
      remove_usuario_tema(cli_addr, i);
    }

  int elem;
  if((elem=buscar_usuario(cli_addr))==-1)
    return -1;
  
  SUSCR *temp = malloc((n_suscr-1) * sizeof(SUSCR));

  memmove(temp, suscr, (elem+1)*sizeof(SUSCR)); 

  memmove(temp+elem, (suscr)+(elem+1), (n_suscr-elem)*sizeof(SUSCR));

  n_suscr--;
  free(suscr);
  suscr = temp;
  return 0;
}
