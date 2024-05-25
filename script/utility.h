#ifndef UTILITY_H
#define UTILITY_H
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>




int my_send(int sd, char* buf, char* err); 
int my_recv(int sd, char* buf, char* err); 


#endif