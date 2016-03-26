/*
   Incluya en este fichero todas las definiciones que pueden
   necesitar compartir todos los módulos (editor, subscriptor y
   proceso intermediario), si es que las hubiera.
*/
#ifndef COMUN_H
#define COMUN_H

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

/* EDITOR -> INTERMEDIARIO */ 
#define GENEV 1
#define CREAT 2
#define ELIMT 3

/* SUSCRIPTOR -> INTERMEDIARIO */
#define NEWSC 4
#define FINSC 5
#define ALTAT 6
#define BAJAT 7

/* INTERMEDIARIO -> SUSCRIPTOR */
#define NOTIF 8
#define NUEVT 9
#define TEMAE 10

/* INTERMEDIARIO -> EDITOR/SUSCRIPTOR */
#define OK 11
#define ERROR 12


#define MAX_REC_SZ 512
#define MAX_CONNECT 10
#define PUERTO_LIBRE 0

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

typedef struct topic_message
{
  int op;
  char tp_nam[64];
  char tp_val[64];
}TOPIC_MSG;

size_t serialize(const TOPIC_MSG* topic, unsigned char** buf);
TOPIC_MSG *deserialize(const unsigned char *buf, const size_t bufSz);
int abrir_puerto_escucha(int port, SOCKADDR_IN *serv_addr);

#endif
