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

typedef struct myStruct
{
  int op;
}MY;

typedef struct topic
{
  char tp_nam[64];
  int mem_sz;
  SOCKADDR_IN *mem;
} TOPIC;


int n_topics = 0;
TOPIC *topics;

int main(){
  /* int tam; */
  /* topics = malloc(sizeof(TOPIC)); */
  /* tam = sizeof(*topics); */
  /* printf("Tamaño de topic sin inicializar subpuntero %d\n", tam); */
  /* topics->mem = malloc(sizeof(SOCKADDR_IN)); */
  /* tam = sizeof(*topics); */
  /* printf("Tamaño de topic con inicializar subpuntero %d\n", tam); */
  /* return 0; */

  int i;
  topics = malloc(sizeof(TOPIC));
  notificar_tema_nuevo("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }
  notificar_tema_nuevo("juan1ro");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }
  notificar_tema_nuevo("apocalipsis");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }

  remove_tema("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }

  notificar_tema_nuevo("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }

  remove_tema("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }

  notificar_tema_nuevo("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }


  remove_tema("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }



  notificar_tema_nuevo("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }



  remove_tema("elmundo");
  printf("Hay %d temas\n", n_topics);
  for(i=0; i<n_topics; i++)
    {
      printf("Tema %d con nombre %s\n", i, topics[i].tp_nam);
    }




  

}

int remove_tema(const char *tema)
{
  int elem;
  if((elem=buscar_tema(tema))==-1)
    return -1;
  
  /* Debemos tener siempre 1 espacio extra */
  TOPIC *temp = malloc((n_topics)*sizeof(TOPIC));
  if(!elem)
    {
      memmove(temp, topics+1, (n_topics-1)*sizeof(TOPIC));
    }
  else
    {
      memmove(temp, topics, (elem)*sizeof(TOPIC));
      memmove(temp+elem, topics+1, (n_topics-elem-1)*sizeof(TOPIC));
    }
  /* int i; */
  /* for(i=0; i<n_topics-1; i++){ */
  /*   temp[i].mem = malloc(sizeof(SOCKADDR_IN)); */
  /* } */
  
  free(topics[elem].mem);
  free(topics);
  n_topics--;
  topics = temp;
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




/* typedef struct topic */
/* { */
/*   char tp_nam[64]; */
/*   int mem_sz; */
/*   MY *mem; */
/* } TOPIC; */


/* int main() */
/* { */
/*   int n_temas = 0; */
/*   TOPIC *temas; */
/*   temas = malloc(sizeof(TOPIC)); */
/*   sprintf(temas[n_temas].tp_nam, "%s", "tema1"); */
/*   temas[n_temas].mem_sz = 1; */
/*   temas[n_temas].mem = malloc(2*sizeof(MY)); */
/*   temas[n_temas].mem[0].op = 11; */
/*   temas[n_temas++].mem[1].op = 111; */
/*   temas = realloc(temas, (1+n_temas)*sizeof(TOPIC)); */
/*   sprintf(temas[n_temas].tp_nam, "%s", "tema2"); */
/*   temas[n_temas].mem_sz = 2; */
/*   temas[n_temas].mem = malloc(2*sizeof(MY)); */
/*   temas[n_temas].mem[0].op = 22; */
/*   temas[n_temas++].mem[1].op = 222; */
/*   temas = realloc(temas, (1+n_temas)*sizeof(TOPIC)); */
/*   sprintf(temas[n_temas].tp_nam, "%s", "tema3"); */
/*   temas[n_temas].mem_sz = 3; */
/*   temas[n_temas].mem = malloc(2*sizeof(MY)); */
/*   temas[n_temas].mem[0].op = 33; */
/*   temas[n_temas++].mem[1].op = 333; */
/*   temas = realloc(temas, (1+n_temas)*sizeof(TOPIC)); */


/*   TOPIC *tmp; */
/*   int elem = 0; */
/*   tmp = malloc((n_temas-1)*sizeof(TOPIC)); */
/*   // copiar la parte anterior al tema */
/*   memmove(tmp, temas, (elem+1)*sizeof(TOPIC));  */

/*   // copiar la parte posterior al tema */
/*   memmove(tmp+elem, (temas)+(elem+1), (n_temas-elem)*sizeof(TOPIC)); */

/*   n_temas--; */
/*   free(temas); */
/*   temas = tmp; */

/*   int i,j; */
/*   for(i=0; i<n_temas; i++){ */
/*     printf("El nuevo array de struct temas: temas[%d] tp_nam:%s mem_sz:%d\n", i, temas[i].tp_nam, temas[i].mem_sz); */
/*     printf("2 miembros\n"); */
/*     for(j=0; j<2; j++){ */
/*       printf("Miembro[%d], op:%d\n", j, temas[i].mem[j].op); */
/*     } */
/*   } */

/*   free(temas); */

/*   return 0; */
/* } */
