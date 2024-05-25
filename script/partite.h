#ifndef PARTITE_H
#define PARTITE_H
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

#define N_OGGETTI 7
#define N_ZAINO 4 

struct Enigmi{
    char domanda[1024]; 
    char risposta[50]; 
};

struct Argomento{
    char nome[20];
    bool bloccato;      //true --> ogg o stanza bloccati
    char descr[1024]; 
    char descr_blocc[1024]; 
    struct Enigmi enigma;   //enigma associato all'oggetto/location 
};

struct Partite{
    char user[50];      //username chiave primaria, non si possono registrare più utenti con lo stesso username
    int scenario;       //in presenza di piu scenari indica quello selezionato 
    char zaino[N_ZAINO][10];      //zaino oggetti raccolti (massimo 4)
    int token;              //token necessari per vincere 
    struct Argomento arg[N_OGGETTI];  //ogni partita ha i suoi dettagli degli oggetti/location perchè possono essere sbloccati o no 
}; 



struct Partite* crea_partita(char* u); 
void inizializza_arg(struct Partite* p_cur, char* s);
void invia_info_partita(struct Partite* partita, int sd); 

#endif