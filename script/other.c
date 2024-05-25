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
#include <termios.h>

#include "utility.h"


#define BUF_SIZE 1024 
#define CMD_SIZE 10
#define SERVER_PORT 4242

char buffer[BUF_SIZE];      //buffer usato per comunicare con il server 
char cmd[CMD_SIZE];         //comando da eseguire inserito dall'utente
int sd_c2, ret; 


/*  Host in cui implemento la funzionalità a piacere.
Il giocatore se sta per terminare il tempo può chiedere solo una volta del tempo aggiuntivo ad un host speciale attraverso la funzione "time", 
la quale andrà a chiedere ad un compagno di digitare 3 numeri i quali saranno inviati al giocatore,
che a scatola chiusa deve scegliere una variabile e il numero che si cela sotto saranno i minuti aggiuntivi 
che il giocatore avrà a disposizione 
*/
int main(int argc, char* argv[]){

    int i, c, x;  
    struct sockaddr_in server_addr; 
    int num1, num2, num3; 
     
    
    sd_c2 = socket(AF_INET, SOCK_STREAM,0);
    if(sd_c2<0){
        perror("Errore creazione socket nel client");
        exit(1);
    }

    //creazione indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); 

    //attendo che il server si metta in ascolto dopo aver digitato start sul server
    for(i=0; i<10; i++){
        ret = connect(sd_c2, (struct sockaddr*)&server_addr, sizeof(server_addr)); 
        if(!ret){
            break; 
        }
        sleep(2);
    }

    if(ret<0){
        perror("errore in fase di connessione"); 
        exit(1); 
    }

    printf("Connessione con il server stabilita! \n\n" );


    printf("Attendi che il giocatore abbia bisogno di te!\n\n"); 
    fflush(stdout);
    fflush(stdin);
    //attendo che il server mandi un comando 
    ret = my_recv(sd_c2, buffer, "richiesta aiuto"); 
    if(ret <= 0){
        exit(1); 
    }

    printf("%s\n", buffer); 

    //svuoto buffer di input per evitare che vengano letti valori precedenti
    tcflush(STDIN_FILENO, TCIFLUSH);
    
    //ciclo finchè non vengono inseriti 3 numeri 
    while(1){

        printf("Digita 3 numeri tra i quali l'utente sceglierà i MINUTI bonus che potrà avere...\n\n");
        memset(buffer,0, sizeof(buffer));

        if(fgets(buffer, BUF_SIZE, stdin) == NULL){
            printf("Errore durante la lettura da stdin\n");
            exit(1);
        }

        if(sscanf(buffer, "%d %d %d", &num1, &num2, &num3) == 3){   //inseriti 3 numeri, esco dal ciclo 
            break; 
        }
        
        printf("Numeri non corretti\n"); 

    }

    //invio dati al server
    ret = my_send(sd_c2, buffer, "numeri host"); 
    if(ret <= 0){
        exit(1);
    }

    printf("Grazie del tuo aiuto, il tempo è fondamentale in una missione.\n"); 
    sleep(3); 

    close(sd_c2);

}