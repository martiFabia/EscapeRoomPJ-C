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

#include "utility.h"
#include "partite.h"
#include "funzioni_cmd.h"


#define BUF_LEN 1024 
#define CMD_LEN 10


int main(int argc, char* argv[]){

    int ret, listener, new_fd, len, porta, i, j; 
    struct sockaddr_in server_addr, client_addr;  
    fd_set master; 
    fd_set read_fds; 
    int fdmax=0; 
    bool server_start=false;    //controllo se server avviato 

    int socket_giocatore;   //socket del giocatore per distinguerlo dall'host speciale
    int socket_host;    //socket host speciale
    int connessioni=0;    //conto quante connessioni ho, per capire se l'host speciale è connesso o no 

    char buffer[BUF_LEN]; 
    char cmd[CMD_LEN]; 
    char arg1[50], arg2[50];    //vettori in cui mi salvo gli argomenti dei comandi 
    char user[50];  //salvo username del giocatore che sta giocando 

  
    struct Partite* partita_cur=NULL;   //puntatore alla partita in corso 
    
    if(argc > 1){
        porta = atoi(argv[1]);
    }else{
        porta = 4242;
    }

    listener = socket(AF_INET, SOCK_STREAM, 0);

    if(listener == -1){
        perror("errore nella creazione del socket"); 
        exit(1); 
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(porta);
    server_addr.sin_addr.s_addr = INADDR_ANY; //mette il server in ascolto su tutte le interfacce

    ret = bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0){
        perror("errore nella bind del server");
        exit(1);
    }

   
    FD_ZERO(&master);
    FD_ZERO(&read_fds); 
    
    FD_SET(STDIN_FILENO, &master); 

    mostra_comandi_start(); 


    while(1){

        //svuoto variabili 
        memset(cmd, 0, sizeof(cmd));
        memset(buffer,0, sizeof(buffer)); 

        read_fds = master; 
        select(fdmax+1, &read_fds, NULL, NULL, NULL); 
        if(ret < 0 ){
            perror("errore nella select");
            exit(1);
        }

        for(i=0; i<= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){

                if(i==STDIN_FILENO){  //comando inserito nella console del server, gestione input 

                    fgets(buffer, BUF_LEN, stdin);

                    if(sscanf(buffer, "%s", cmd) == 1){    //mi aspetto un comando  
                        if(!strcmp(cmd, "start")){  //comando START

                            if(server_start){
                                printf("Server già avviato... \n"); 
                                continue; 
                            }

                            server_start = true; 
                            printf("Avvio server\n");
                            ret = listen(listener, 2);
                            if(ret == -1){
                                perror("Errore nella listen del server");
                                exit(1);
                            }
                            FD_SET(listener, &master);  //agg listener tra i socket monitorati
                            fdmax=listener;

                        }else if(!strcmp(cmd, "stop")){   //comando STOP
                            if(!server_start){
                                printf("Server non in funzione\n");
                                continue; 
                            }

                            if(partita_cur != NULL){
                                //c'è una partita in corso, il server non può essere arrestato
                                printf("Partita in corso. Il server non può essere arrestato\n"); 
                                continue; 
                            }

                            //gestisco arresto server

                            close(listener);    
                            //chiudo tutte le connessioni che ci sono 
                            for (j = 0; j <= fdmax; j++) {
                                if (FD_ISSET(j, &master)) {
                                    if (j != listener) {
                                        close(j);
                                        FD_CLR(j, &master);
                                    }
                                }
                            }
                            server_start=false;
                            connessioni=0; 
                            printf("Disconnessione server\n");
                            exit(0);    

                        }else { //il comando non è riconosciuto
                            printf("Comando inesistente. Riprova\n\n"); 
                            mostra_comandi_start(); 
                        }
                    }

                }else if(i==listener){        //ho ricevuto richiesta di connessione

                    len = sizeof(client_addr); 
                    new_fd = accept(listener, (struct sockaddr*)&client_addr, &len); 
                    if(new_fd < 0){
                        perror("errore accept del server\n"); 
                        close(new_fd); 
                        continue; 
                    }

                    printf("Connessione accettata\n");
                    connessioni++;  
                    FD_SET(new_fd, &master);    //inserisco il nuovo socket tra i master 

                    if(new_fd > fdmax){
                        fdmax=new_fd;   //aggiorno fdmax 
                    }
                }else {
                    //ho ricevuto un comando dal client (socket di comunicazione)

                    ret = my_recv(i, buffer, "ricezione comando"); 
                    if(ret == -1){ 
                        continue; 
                    }
                    if(ret == 0){
                        //Qua arrivo se l'utente ha digitato il comando end, se ha perso o se ha vinto e dunque ha chiuso la connessione
                        //Lo tolgo correttamente dal set e chiudo il socket.
                        printf("Socket del client chiuso, chiudo il socket %d.\n", i);
                        close(i);
                        FD_CLR(i, &master);

                        if(i==socket_giocatore){    //il socket chiuso è quello del giocatore e non dell'host speciale 
                            //deallocazione partita 
                            printf("Deallocazione struttura partita\n"); 
                            free(partita_cur);
                            partita_cur = NULL; 
                        }
                       
                        connessioni--;  
                        printf("Chiusura socket %d avvenuta con successo\n", i);
                        continue; 
                    }

                    //la recv è andata a buon fine, metto in evidenza il comando ricevuto e gli argomenti e scelgo che funzione eseguire 
                    sscanf(buffer, "%s %s %s", cmd, arg1, arg2);

                    if(!strcmp(cmd, "register") || !strcmp(cmd, "login")){   //arg1 è l'username, arg2 la password 

                        socket_giocatore = i;   //salvo il socket del giocatore quando accede 
                        memset(buffer,0, sizeof(buffer));
                        if(!comando_register(arg1, arg2, cmd)){
                            //registrazione o login falliti
                            printf("Autentificazione fallita\n"); 
                            strcpy(buffer, "err"); 
                            ret = my_send(i, buffer, "invio errore registrazione");
                            if(ret == -1){
                                exit(1); 
                            } 
                        }else{
                            
                            strcpy(user, arg1);     //mi salvo username del giocatore che sta accedendo al gioco 
                            memset(buffer,0, sizeof(buffer));
                            strcpy(buffer, "ok"); 
                            ret = my_send(i, buffer, "invio ok registrazione"); 
                            if(ret == -1){
                                exit(1);
                            }
                        }

                        continue; 
                    }                   
                    if(!strcmp(cmd, "start")){  //arg1 è lo scenario scelto 
                        //creo nuova partita 
                        printf("Gestione comando start--> creazione nuova partita\n"); 
                        partita_cur = comando_start(user, arg1); 
                        continue; 
                    }


                    if(!strcmp(cmd, "look")){   //arg1 è l'oggetto o la stanza da esplorare 
                        printf("Gestione comando look\n"); 
                        comando_look(partita_cur, arg1, i); 
                    }

                    if(!strcmp(cmd, "take")){   //arg1 è l'oggetto che vuole raccogliere 
                        printf("Gestione comando take\n"); 
                        comando_take(partita_cur, arg1, i); 
                    }

                    if(!strcmp(cmd, "use")){   //serve sia arg1 che arg2 
                        printf("Gestione comando use\n"); 
                        comando_use(partita_cur, arg1, arg2, i); 
                    }

                    if(!strcmp(cmd, "obj")){   //non abbiamo argomenti  
                        printf("Gestione comando obj\n"); 
                        comando_obj(partita_cur, i); 
                    }

                    if(!strcmp(cmd, "time")){    //non abbiamo argomenti 
                        printf("Gestione comando time\n");
                        //individuo il socket dell'host speciale 
                        if(connessioni < 2){
                            printf("Host speciale non connesso\n"); 
                            memset(buffer,0, sizeof(buffer));
                            strcpy(buffer, "err"); 
                            ret = my_send(i, buffer, "invio errore connessione host speciale");
                            if(ret == -1){
                                exit(1); 
                            }
                            continue; 
                        }
                        if(socket_giocatore == fdmax){
                            socket_host = socket_giocatore -1; 
                        }else {
                            socket_host = fdmax; 
                        }
                        comando_time(socket_giocatore, socket_host, partita_cur);
                    }

                }

            }
        } //fine for che scorre i descrittori 

    } //fine while(1)

}
