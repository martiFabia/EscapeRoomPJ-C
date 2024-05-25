all: client server other

client: client.o utility.o
		gcc -Wall client.o utility.o -o client

server: server.o utility.o partite.o funzioni_cmd.o 
		gcc -Wall server.o utility.o partite.o funzioni_cmd.o -o server

other: other.o utility.o
		gcc -Wall other.o utility.o -o other

clean: 
		rm *.o client server other

utility.o: utility.c utility.h 
		gcc -Wall -c  utility.c -o utility.o

partite.o: partite.c partite.h
		gcc -Wall -c partite.c -o partite.o

funzioni_cmd.o: funzioni_cmd.c funzioni_cmd.h 
		gcc -Wall -c  funzioni_cmd.c -o funzioni_cmd.o


		