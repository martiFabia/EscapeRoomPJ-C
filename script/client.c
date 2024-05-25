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


#define BUF_SIZE 1024 
#define CMD_SIZE 10
#define SERVER_PORT 4242

char buffer[BUF_SIZE];      //buffer usato per comunicare con il server 
char cmd[CMD_SIZE];         //comando da eseguire inserito dall'utente
int sd_c, ret; 

void look_vuoto(); 
void mostra_comandi(); 
void mostra_info_partita(int tempo);
void gestione_partita(); 

int main(int argc, char* argv[]){

    int i, x; 
    struct sockaddr_in server_addr; 
     
    int scenario; 

    bool accesso =false;  //controllo se il login/signup è andato a buon fine 
    bool start = false;   //controllo che venga scelto lo scenario e il gioco ha inizio 

    sd_c = socket(AF_INET, SOCK_STREAM,0);
    if(sd_c<0){
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
        ret = connect(sd_c, (struct sockaddr*)&server_addr, sizeof(server_addr)); 
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


    //connessione avvenuta, inizia scambio messaggi con il server
    //registrazione/login 
    while( !accesso ){
        memset(buffer,0, sizeof(buffer));

        printf("Per iniziare identificati! \nDigita:\nlogin username password--> se fai già parte della community \nregister username password -- >se sei un nuovo utente\n\n");
        fgets(buffer, BUF_SIZE, stdin);

        sscanf(buffer, "%s", cmd);
        if(!strcmp(cmd, "register") || !strcmp(cmd, "login")){  //controllo che il comando sia valido
            //invio dati al server
            ret = my_send(sd_c, buffer, "dati login"); 
            if(ret <= 0){
                exit(1);
            }

            ret = my_recv(sd_c, buffer, "risposta esito login"); 
            if(ret <= 0){
                exit(1); 
            }

            if(!strcmp(buffer, "err")){     //se ricevo err vuoldire che non è stato possibile accedere
                printf("\nAutentificazione fallita. Riprova\n\n");
            }else {
                accesso =true; 
            }
        }else {
            printf("Comando non valido. Controlla bene\n"); 
        }
    }

    //accesso effettuato
    //scelta scenario e inzio gioco 
    while(!start){
        memset(buffer,0, sizeof(buffer));
        printf("\n************************** ESCAPE ROOM *******************************\n\n");
        printf("Ci siamo quasi! Scegli lo scenario che preferisci.\nDigita--> start numero scenario\n CHE LA MAGIA ABBIA INIZIO!\n");
        printf("1- Museo Egizio\n2- Coming soon... \n"); 
        fgets(buffer, BUF_SIZE, stdin);

        sscanf(buffer, "%s %d", cmd, &scenario);
        if(!strcmp(cmd, "start") && scenario == 1){
            //invio comando al server
            ret = my_send(sd_c, buffer, "scelta scenario"); 
            if(ret == -1){
                exit(1);
            }

            start = true; 
        }else if(!strcmp(cmd, "start") && scenario == 2){

            printf("Escape room in allestimento...\n"); 

        }else{
            printf("Comando non valido.Controlla bene\n");
        }
        
    }


    //arrivati qua inizia il gioco vero e proprio
    if(scenario == 1){
        printf("\n************************** MUSEO EGIZIO *******************************\n\n");
        printf("Benvenuto nel Museo Egizio, un luogo avvolto dall'antichità e ricco di misteri.\nLa tua missione è audace e pericolosa: rubare la corona d'oro appartenuta al grande faraone Ramses II.\nATTENZIONE: hai solo 10 minuti!\n");
        look_vuoto(); 
        gestione_partita(); 

    }

    //il client esegue questo codice in seguito al comando "end", se perde perchè scade il tempo o se vince 
    //chiudo connessione
    close(sd_c); 
    exit(0);

}



void gestione_partita(){
    char arg1[50], arg2[50];    //argomenti dei comandi che l'utente puo inserire
    int tempo_rimasto; 
    char scelta[10];   //variabile per memorizzare i minuti bonus 
    bool time_usato=false;      //la funzione speciale time puo essere usata solo una volta

    //la partita dura 10 minuti, se il tempo finisce, l'utente perde
    time_t fine_partita, current_time;

    fine_partita = time(NULL) + 600;    //ora corrente + 10 minuti 

//ciclo che attende inserimento da tastiera dei comandi 
    while(1){

        //'pulisco' sempre le variabili
        memset(buffer, 0, sizeof(buffer));
        memset(cmd, 0, sizeof(cmd));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));

        fgets(buffer, BUF_SIZE, stdin);

        //prima di gestire eventuali comandi, devo sempre controllare che il tempo non sia scaduto
        time(&current_time);
        tempo_rimasto = fine_partita - current_time; 
        if(tempo_rimasto <= 0){
            printf("Tempo scaduto. Ti aspettiamo per una nuova missione insieme\n");
            sleep(2);
            break;
        }

        ret = sscanf(buffer, "%s %s %s", cmd, arg1, arg2);  //separo il comando dai due argomenti inseriti, mettendoli nelle adeguate variabili

        if(!strcmp(cmd, "look") && ret == 1){   //look senza argomenti 
            look_vuoto(); 
            continue; 

        }else if(!strcmp(cmd, "look") && ret == 2){   //look con un argomento 
            //invio comando al server 
            ret = my_send(sd_c, buffer, "comando look"); 
            if(ret == -1){
                exit(1);
            } 
            memset(buffer, 0, sizeof(buffer));        
            //attendo la risposta  (descrizione oggetto/location)
            ret = my_recv(sd_c, buffer, "risposta comando look"); 
            if(ret == -1){
                exit(1); 
            } 

            printf("%s", buffer);

        }else if(!strcmp(cmd, "take") && ret == 2){
            //invio comando al server
            ret = my_send(sd_c, buffer, "comando take"); 
            if(ret == -1){
                exit(1);
            } 
            memset(buffer, 0, sizeof(buffer));        
            //attendo la risposta  
            ret = my_recv(sd_c, buffer, "risposta comando take"); 
            if(ret == -1){
                exit(1); 
            } 

            if(strncmp(buffer, "EN", 2) == 0){
                //se la risposta inizia con EN, il client ha ricevuto un enigma--> l'utente deve immettere la risposta 
                memmove(buffer, buffer + 2, strlen(buffer) - 1);   //rimuovo EN shiftando la stringa di due caratteri a sx
                //stampo esito take (enigma)
                printf("%s", buffer);
                memset(buffer, 0, sizeof(buffer));

                //utente digita risposta
                fgets(buffer, BUF_SIZE, stdin);
            
                // Mando la risposta dell'enigma al server
                ret = my_send(sd_c, buffer, "risposta enigma"); 
                if(ret == -1){
                    exit(1);
                } 
                memset(buffer, 0, sizeof(buffer));        
                //attendo esito risposta enigma
                ret = my_recv(sd_c, buffer, "istruzioni post enigma"); 
                if(ret == -1){
                    exit(1); 
                } 

                if(strncmp(buffer, "WIN", 3) == 0){     //l'utente ha risposto correttamente all'ultimo enigma, ha vinto 
                    memmove(buffer, buffer + 3, strlen(buffer) - 1);   //rimuovo WIN shiftando la stringa di tre caratteri a sx
                    printf("%s", buffer); 
                    printf("Ti aspettimo per una nuova missione insieme. A presto!\n\n"); 
                    break; 
                }
                //stampo esito enigma
                printf("%s", buffer); 
            }else {
                //altrimenti se non è un enigma, stampo esito take e continuo 
                printf("%s", buffer);  
            }
        }else if(!strcmp(cmd, "take") && (ret == 1 || ret == 3)){ //se viene scritto il comando take senza argomenti o 2 argomenti 
            printf("Comando non corretto.\nInserisci UN argomento valido di un oggetto che vuoi raccogliere o sbloccare!\n");
            continue; 

        }else if(!strcmp(cmd, "use")){
            if(ret == 1 || ret == 2){   //non ho casi in cui usare use con solo 1 oggetto 
                printf("Comando non completo.\nInserisci 2 argomenti validi\n");
                continue; 
            }
            if(ret == 3){   //unica use possibile --> use amuleti pergamena
                if(!strcmp(arg1, "amuleti") && !strcmp(arg2, "pergamena")){
                    //invio comando al server
                    ret = my_send(sd_c, buffer, "comando use"); 
                    if(ret == -1){
                        exit(1);
                    } 
                    memset(buffer, 0, sizeof(buffer));        
                    //attendo la risposta  
                    ret = my_recv(sd_c, buffer, "risposta comando use"); 
                    if(ret == -1){
                        exit(1); 
                    } 

                    printf("%s", buffer);
 
                }else{
                    printf("Combinazione oggetti sbagliata. Riprova\n");
                    continue;
                }

            }
        }else if(!strcmp(cmd, "obj")){
            if(ret > 1){    //sono stati inseriti degli argomenti, quindi è sbagliato
                printf("Il comando obj non vuole argomenti. Riprova\n");  
                continue;

            }else { //comando corretto
                //invio comando al server
                ret = my_send(sd_c, buffer, "comando obj"); 
                if(ret == -1){
                    exit(1);
                } 
                memset(buffer, 0, sizeof(buffer));        
                //attendo la risposta  
                ret = my_recv(sd_c, buffer, "risposta comando obj"); 
                if(ret == -1){
                    exit(1); 
                } 
                
                printf("%s", buffer); 

            }
        }else if(!strcmp(cmd, "end")){
            if(ret > 1){    //sono stati inseriti degli argomenti, quindi è sbagliato
                printf("Il comando end non vuole argomenti. Riprova\n");  
                continue; 

            }else{  //comando corretto
                printf("Partita conclusa. Ti aspettiamo per una nuova missione insieme\n\n");
                break;
            }
        }else if(!strcmp(cmd, "time")){
            if(ret > 1){    //sono stati inseriti degli argomenti, quindi è sbagliato
                printf("Il comando time non vuole argomenti. Riprova\n");  
                continue; 
            }
            if(time_usato){
                printf("Hai già utilizzato la funzione speciale mi dispiace.\n"); 
                continue; 
            }
            ret = my_send(sd_c, buffer, "comando time"); 
            if(ret == -1){
                exit(1);
            } 
            memset(buffer, 0, sizeof(buffer));        
            //attendo la risposta con i 3 pacchi da scegliere
            ret = my_recv(sd_c, buffer, "risposta comando time"); 
            if(ret == -1){
                exit(1); 
            }  

            if(!strcmp(buffer, "err")){     //il server ha inviato errore 
                printf("Host speciale non connesso. Impossibile usare funzione speciale\n");
                continue; 
            }
            
            printf("%s\n", buffer); 

            memset(buffer, 0, sizeof(buffer));
            //utente digita il pacco scelto 
            fgets(buffer, BUF_SIZE, stdin);
        
            // Mando la scelta al server
            ret = my_send(sd_c, buffer, "scelta pacco"); 
            if(ret == -1){
                exit(1);
            } 
            memset(buffer, 0, sizeof(buffer));        
            //attendo esito
            ret = my_recv(sd_c, scelta, "esito pacco scelto"); 
            if(ret == -1){
                exit(1); 
            } 

            sprintf(buffer, "Hai ricevuto %s minuti bonus!\n", scelta); 
            time_usato = true; 
            tempo_rimasto = tempo_rimasto + (atoi(scelta)*60);  //aggiungo il tempo bonus (scelta sono minuti e li trasformo in secondi)
            fine_partita = fine_partita +  (atoi(scelta)*60);   //aggiorno anche l'orario di fine partita
            printf("%s\n", buffer); 

        }else {
            mostra_comandi();
            continue;
        }

        printf("\n****************************************************************************\n"); 
        mostra_info_partita(tempo_rimasto); 
        printf("*****************************************************************************\n\n"); 

    }//fine while(1)
    
}

void look_vuoto(){

    printf("\nLa prima sala in cui ti trovi accoglie statue colossali e sculture raffiguranti antichi faraoni e dee egizie.\nVasi canopi, oggetti di culto e **amuleti** luminosi sono disposti su altari.\nDavanti a te nell'oscurità intravedi due porte: ++porta1++ e ++porta2++.\nA causa del buio non riesci a leggere i nomi delle stanze.\n");
    printf("\n***********************************************************************\n\n");
}

void mostra_comandi(){
    printf("\n***********************************************************************\n");
    printf("Puoi utilizzare uno di questi comandi:\n");
    printf("--> look [location | object], per ottenere una desrizione del soggetto\n");
    printf("--> take object, per mettere un oggetto nello zaino\n");
    printf("--> use object1 object2, per utilizzare un oggetto su un'altro oggetto\n");
    printf("--> obj, per visualizzare gli oggetti raccolti nello zaino\n");
    printf("--> end, per terminare il gioco\n");
    printf("--> time, per utilizzare funzione speciale\n\n");
    
}

void mostra_info_partita(int tempo){
    // Devo mostrare numero di token raccolti, token mancanti e il tempo rimanente
    //il server concatena già tutte queste info, eccetto il tempo che è gestito dal client 
    char info_tempo[50]; 
    memset(buffer, 0, sizeof(buffer)); 

    ret = my_recv(sd_c, buffer, "risposta info sulla partita"); 
    if(ret == -1){
        exit(1); 
    } 

    sprintf(info_tempo, " Tempo rimanente: %d minuti", (long long)(tempo/60));
    strcat(buffer, info_tempo);     //concateno le info ricevute dal server con il tempo rimasto

    printf("%s\n", buffer); 

}


