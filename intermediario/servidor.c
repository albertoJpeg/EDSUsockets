#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "message.h"
#include <netinet/in.h>
#include <arpa/inet.h>

/* Función auxiliar que escribe un número de puerto en el fichero */
void escribir_puerto (int puerto);
/* Esta función se encarga de abrir el socket UDP */
int abrir_socket_udp(void);
/* Esta función se encarga de abrir el socket TCP */
int abrir_socket_tcp(void);
/* Esta función se encarga de cambiar el tipo de operación de un mensaje */
void cambiar_peticion(UDP_Msg* mensaje, int operacion);
/* Esta función se encarga de enviar el fichero por el puerto indicado */
void enviar_fichero(char* fichero, int fd);

int PUERTO_UDP, PUERTO_TCP;
struct sockaddr_in dir_udp_serv, dir_udp_client, dir_tcp_serv, dir_tcp_client;

/* FUNCION MAIN DEL PROGRAMA SERVIDOR */
int main(int argc,char* argv[]){

  UDP_Msg mensaje;
  int socket_tcp, socket_udp, tam_dir, fd;
  tam_dir=sizeof(struct sockaddr_in);

  /* Creacion del socket UDP */
  /* Asignacion de la direccion local (del servidor) Puerto UDP*/
  socket_udp=abrir_socket_udp();

  /* Escribimos el puerto de servicio */
  escribir_puerto(PUERTO_UDP);
  
  /* Creacion del socket TCP de servicio */
  socket_tcp=abrir_socket_tcp();

  while(1)
    { /* Bucle de procesar peticiones */
      fprintf(stdout,"SERVIDOR: Esperando mensaje.\n");

      /* Recibo mensaje */
      if(recvfrom(socket_udp,(char*) &mensaje, sizeof(UDP_Msg), 0, (struct sockaddr*) &dir_udp_client, (socklen_t*) &tam_dir) < 0)
	{
	  fprintf(stdout,"SERVIDOR: Mensaje del cliente: ERROR\n");
	  close(socket_udp);
	  close(socket_tcp);
	  exit(1);
	}
      else
	{
	  fprintf(stdout,"SERVIDOR: Mensaje del cliente: OK\n");
	} 

      if(ntohl(mensaje.op)==QUIT)
	{/* Mensaje QUIT*/
	  fprintf(stdout,"SERVIDOR: QUIT\n");
	  cambiar_peticion(&mensaje, OK);
	  if(sendto(socket_udp,(char*) &mensaje, sizeof(UDP_Msg), 0, (struct sockaddr*) &dir_udp_client, tam_dir)<0)
	    {
	      fprintf(stdout,"SERVIDOR: Enviando del resultado [OK]: ERROR\n");  
	      close(socket_udp); 
	      exit(1);
	    }
	  else
	    {
	      fprintf(stdout,"SERVIDOR: Enviando del resultado [OK]: OK\n");  
	      break;
	    }
	}
      else
	{
	  fprintf(stdout,"SERVIDOR: REQUEST(%s,%s)\n", mensaje.local, mensaje.remoto);
	  /* Envio del resultado */
	  if(access(mensaje.remoto, F_OK) < 0)
	    { //Comprobamos si existe el fichero en el servidor
	      cambiar_peticion(&mensaje, ERROR);
	      if(sendto(socket_udp,(char*) &mensaje, sizeof(UDP_Msg), 0, (struct sockaddr*) &dir_udp_client, tam_dir)<0)
		{
		  fprintf(stdout,"SERVIDOR: Enviando del resultado [ERROR]: ERROR\n");  
		  close(socket_udp); 
		  exit(1);
		}
	      else
		{
		  fprintf(stdout,"SERVIDOR: Enviando del resultado [ERROR]: OK\n");  
		}
	    }
	  else
	    {
	      cambiar_peticion(&mensaje, OK);
	      mensaje.puerto=PUERTO_TCP;
	      if(sendto(socket_udp,(char*) &mensaje, sizeof(UDP_Msg), 0, (struct sockaddr*) &dir_udp_client, tam_dir)<0)
		{
		  fprintf(stdout,"SERVIDOR: Enviando del resultado [OK]: ERROR\n");
		  close(socket_udp); 
		  exit(1);
		}
	      else
		{
		  fprintf(stdout,"SERVIDOR: Enviando del resultado [OK]: OK\n");
		  /* Esperamos la llegada de una conexion */
		  if ((fd=accept(socket_tcp,(struct sockaddr*) &dir_tcp_client, (socklen_t*) &tam_dir)) < 0)
		    {
		      fprintf(stdout,"SERVIDOR: Llegada de un mensaje: ERROR\n");
		      close(socket_tcp);
		      exit(1);
		    }
		  else
		    {
		      fprintf(stdout,"SERVIDOR: Llegada de un mensaje: OK\n");
		      enviar_fichero(mensaje.remoto, fd);
		      close(fd);
		    } 
		}
	    }	
	}
    }
  fprintf(stdout,"SERVIDOR: Finalizado\n");
  close(socket_udp);
  close(socket_tcp);
  return 0;
}

int abrir_socket_udp(){
  int socket_udp; /* Socket (de tipo UDP) de trasmision de ordenes */
  int tam_dir=sizeof(struct sockaddr_in);
  
  /* Creacion del socket UDP */
  socket_udp=socket(AF_INET,SOCK_DGRAM,0);
  if(socket_udp<0){
    fprintf(stdout,"SERVIDOR: Creacion del socket UDP: ERROR\n");
    exit(1);
  }else{
    fprintf(stdout,"SERVIDOR: Creacion del socket UDP: OK\n");
  }
  
  /* Inicialización del struct UDP */
  bzero((char*)&dir_udp_serv, sizeof(struct sockaddr_in)); /* Pone a 0 el struct */

  dir_udp_serv.sin_family=AF_INET;
  dir_udp_serv.sin_addr.s_addr=inet_addr(HOST_CLIENTE);/*INADDR_ANY;*/
  dir_udp_serv.sin_port=htons(0);

  if(bind(socket_udp, (struct sockaddr*)&dir_udp_serv, sizeof(struct sockaddr_in)) < 0){
    fprintf(stdout,"SERVIDOR: Asignacion del puerto servidor: ERROR\n");
    close(socket_udp);
    exit(1);
  }else{
    fprintf(stdout,"SERVIDOR: Asignacion del puerto servidor: OK\n");
  }
  
  if(getsockname(socket_udp, (struct sockaddr*)&dir_udp_serv, (socklen_t*) &tam_dir) < 0){
    fprintf(stdout,"SERVIDOR: Puerto UDP reservado: ERROR\n"); 
    close(socket_udp);
    exit(1);
  }

  PUERTO_UDP=ntohs(dir_udp_serv.sin_port);
  return socket_udp;
}

int abrir_socket_tcp(){
  int socket_tcp; /* Socket (de tipo TCP) para transmision de datos */
  int tam_dir=sizeof(struct sockaddr_in);
    
  /* Creacion del socket TCP */
  socket_tcp=socket(AF_INET,SOCK_STREAM,0);
  if(socket_tcp<0){
    fprintf(stdout,"SERVIDOR: Creacion del socket TCP: ERROR\n");
    exit(1);
  }else{
    fprintf(stdout,"SERVIDOR: Creacion del socket TCP: OK\n");
  }

  /* Inicialización del struct TCP */
  bzero((char*)&dir_tcp_serv,sizeof(struct sockaddr_in)); /* Pone a 0 el struc */

  dir_tcp_serv.sin_family=AF_INET;
  dir_tcp_serv.sin_addr.s_addr=inet_addr(HOST_CLIENTE);/*INADDR_ANY;*/
  dir_tcp_serv.sin_port=htons(0);
    
  if(bind(socket_tcp, (struct sockaddr*)&dir_tcp_serv, sizeof(struct sockaddr_in)) < 0){
    fprintf(stdout,"SERVIDOR: Asignacion del puerto servidor: ERROR\n");
    close(socket_tcp);
    exit(1);
  }else{
    fprintf(stdout,"SERVIDOR: Asignacion del puerto servidor: OK\n");
  }

  if(listen(socket_tcp,2)<0){
    fprintf(stdout,"SERVIDOR: Aceptacion de peticiones: ERROR\n");
    close(socket_tcp); 
    exit(1);
  }else{
    fprintf(stdout,"SERVIDOR: Aceptacion de peticiones: OK\n");
  }
  
  if(getsockname(socket_tcp, (struct sockaddr*)&dir_tcp_serv, (socklen_t*) &tam_dir)<0){
    fprintf(stdout,"SERVIDOR: Puerto TCP reservado: ERROR\n"); 
    close(socket_tcp);
    exit(1);
  }else{
    fprintf(stdout,"SERVIDOR: Puerto TCP reservado: OK\n");  
  }

  PUERTO_TCP=dir_tcp_serv.sin_port;
  return socket_tcp;
}


void cambiar_peticion(UDP_Msg *mensaje, int operacion){
  mensaje->op=htonl(operacion); /* El mensaje mandado es de tipo "operacion" */
}

void enviar_fichero(char* fichero, int fd){
  char buff[512];
  size_t tam;
  int fich;
  if((fich=open(fichero, O_RDONLY))<0){
    fprintf(stdout, "Error en lectura de fichero");
    exit(1);
  }
  while((tam=read(fich, (void*)buff, 512))>0)
    send(fd,(void*)buff,tam,0);

  close(fich);
}


void escribir_puerto(int puerto){
  int fd;
  if((fd=creat(FICHERO_PUERTO,0660))>=0){
    write(fd,&puerto,sizeof(int));
    close(fd);
    fprintf(stdout,"SERVIDOR: Puerto guardado en fichero %s: OK\n",FICHERO_PUERTO);
  }
}
