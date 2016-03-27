CFLAGS=-Wall -g
CC=gcc

all: intermediario

intermediario.o: comun.h
comun.o: comun.h

intermediario: intermediario.o comun.o
	$(CC) $(CFLAGS) -o $@ intermediario.o comun.o -lpthread

clean:
	rm -f *.o intermediario
