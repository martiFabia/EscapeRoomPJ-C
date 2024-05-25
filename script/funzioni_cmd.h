#ifndef FUNZIONICMD_H
#define FUNZIONICMD_H
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

#include "utility.h"
#include "partite.h"

void mostra_comandi_start(); 
struct Partite* comando_start(char* user, char* s);
int trova_ogg(struct Partite* partita, char* obj);
bool comando_register(char* user, char* psw, char* cmd); 

void comando_look(struct Partite* partita, char* arg, int sd);
void comando_take(struct Partite* partita, char* obj, int sd);
void comando_use(struct Partite* partita, char* arg1, char* arg2, int sd);
void comando_obj(struct Partite* partita, int sd);
void comando_time(int sd_player, int sd_host, struct Partite* p); 


#endif