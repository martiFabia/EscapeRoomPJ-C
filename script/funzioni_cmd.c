#include "funzioni_cmd.h"


int ret; 
char buf[1024];     //buffer per inviare/ricevere messaggi 
int zaino;          //variabile che conta gli oggetti nello zaino

void mostra_comandi_start(){

    printf("****************SERVER STARTED******************\n");
    printf("Digita un comando:\n");
    printf("1) start --> avvia il server di gioco\n");
    printf("2) stop --> termina il server\n");
    printf("************************************************\n");
}

struct Partite* comando_start(char* user, char* s){
    
    struct Partite* p_cur; 
    p_cur = crea_partita(user); 
    
   //inizializzo variabili
    zaino=0; 
    p_cur->scenario = atoi(s);      //scenario scelto dall'utente
    inizializza_arg(p_cur, s);  //inizializzo location e oggetti
    return p_cur; 

}

bool comando_register(char* user, char* psw, char* cmd){
    fflush(stdout);
    FILE* fd;
    int trovato=0;
    char u[50], p[50];

    fd=fopen("utenti.txt", "r+");
    if(fd == NULL){
        perror("errore apertura file"); 
        return false; 
    }

	//il while continua a ciclare finché sul file la fscanf riesce a leggere coppie utente-password
    while(fscanf(fd, "%s %s", u, p)==2){
        if(!strcmp(u, user)){
            trovato=1;
            break;  
        }
    }

    if(trovato==1 && !strcmp(cmd, "login") && !strcmp(p, psw)){
        fclose(fd);
		//utente gia registrato, sta facendo login 
        printf("Login effettuato. Ciao %s\n", user);
        return true;
    }else if(trovato == 0 && !strcmp(cmd, "register") ) {
        //nuovo utente, lo registro 
        fprintf(fd, "%s  %s\n", user, psw); //aggiungo utente al file 
        fclose(fd);
		printf("Registrazione eseguita. Benvenuto %s \n", user);
        return true; 
    }

    return false;   //registrazione o login errati 
}
    

//funzione che restituisce l'indice dell'oggetto che si sta cercando se esiste, altrimenti -1
int trova_ogg(struct Partite* partita, char* obj){
    int i; 
    for(i=0; i< N_OGGETTI; i++){
        if(!strcmp(obj, partita->arg[i].nome)){
            return i; 
        }
    }

    return -1; 
}

void comando_look(struct Partite* partita, char* arg, int sd){

    //devo verificare se l'argomento è corretto ed esiste tra quelli presenti nel gioco
    int oggetto = trova_ogg(partita, arg);; 
    
    if(oggetto == -1){    //controllo se argomento valido
        strcpy(buf, "Argomento errato. Riprova con oggetto o location valida\n"); 
    }else{
        //controllo se l'oggetto richiesto è bloccato o no
        if(partita->arg[oggetto].bloccato){
            strcpy(buf, partita->arg[oggetto].descr_blocc); 
        }else {
            strcpy(buf, partita->arg[oggetto].descr); 
        }
    }

    ret = my_send(sd, buf, "invio risposta cmd look"); 
    if(ret == -1){
        exit(1); 
    }
    invia_info_partita(partita, sd); 

}



void comando_take(struct Partite* partita, char* obj, int sd){

    int i; 
    bool vittoria=false; 
    int oggetto = trova_ogg(partita, obj);

    if(oggetto == -1){    //controllo se argomento valido

        strcpy(buf, "Argomento errato. Riprova con oggetto o location valida\n"); 

    }else if(!strcmp(obj, "amuleti")){    //oggetto bloccato da domanda su Cleopatra
        if(partita->arg[oggetto].bloccato){
            //amuleti bloccati, invio domanda
            strcpy(buf, partita->arg[oggetto].enigma.domanda); 

            ret = my_send(sd, buf, "invio enigma amuleti"); 
            if(ret == -1){
                exit(1); 
            }
            memset(buf, 0, sizeof(buf));   
            //attendo risposta 
            ret = my_recv(sd, buf, "risposta enigma amuleti");
            if(ret == -1){
                exit(1); 
            }
            if(!strcmp(partita->arg[oggetto].enigma.risposta, buf)){    //risposta corretta 
                partita->arg[oggetto].bloccato = false; 
                strcpy(buf, "\nRisposta esatta! Ora puoi raccogliere l'oggetto\n");
            }else{
                strcpy(buf, "\nRisposta sbagliata :(\n(ATT: la lettera deve essere digitata maiuscola, senza spazi a seguire)\n");
            }

        }else { //oggetto sbloccato va inserito nello zaino 
            for(i=0; i< N_ZAINO; i++){
                if(!strcmp(partita->zaino[i], "amuleti")){
                    strcpy(buf, "--> Amuleti già raccolti\n");
                    break; 
                }
                if(!strcmp(partita->zaino[i], "\0")){
                    strcpy(partita->zaino[i], obj); 
                    strcpy(buf, "--> Amuleti messi nello zaino correttamente.\nProsegui l'avventura!\n");
                    zaino++;  
                    break; 
                }
            }
            if(i == N_ZAINO){     //zaino pieno, applico politica FIFO 
                //amuleti sostituiscono l'oggetto inserito per primo
                strcpy(partita->zaino[(zaino%4)], obj); 
                strcpy(buf, "--> Amuleti messi nello zaino correttamente.\nProsegui l'avventura!\n");
                zaino++; 
            }
        }
    }else if(!strcmp(obj, "porta2")){     //stanza bloccata da parole con lettere mancanti
        if(partita->arg[oggetto].bloccato){  
            //porta bloccata, invio domanda
            strcpy(buf, partita->arg[oggetto].enigma.domanda); 

            ret = my_send(sd, buf, "invio enigma porta2"); 
            if(ret == -1){
                exit(1); 
            }
            memset(buf, 0, sizeof(buf));   
            //attendo risposta 
            ret = my_recv(sd, buf, "risposta enigma porta2");
            if(ret == -1){
                exit(1); 
            }
            if(!strcmp(partita->arg[oggetto].enigma.risposta, buf)){    //risposta corretta, stanza viene sbloccata e token preso
                partita->arg[oggetto].bloccato = false; 
                partita->token++; 
                strcpy(buf, "\nStanza sbloccata. Hai conquistato un token.\nComplimenti! Ora puoi entrare\n");
            }else{
                strcpy(buf, "\nRisposta sbagliata :( \n");
            }

        }else{
            //porta già sbloccata
            strcpy(buf, "\nStanza già sbloccata. Puoi entrare!\n"); 
        }
        
    }else if(!strcmp(obj, "vetrina")){    //bloccata da un lucchetto con codice a 4 cifre
        if(partita->arg[oggetto].bloccato){  
            //vetrina bloccata, invio domanda
            strcpy(buf, partita->arg[oggetto].enigma.domanda);

            ret = my_send(sd, buf, "invio domanda vetrina"); 
            if(ret == -1){
                exit(1); 
            }
            memset(buf, 0, sizeof(buf));   
            //attendo risposta 
            ret = my_recv(sd, buf, "risposta codice vetrina");
            if(ret == -1){
                exit(1); 
            }

            if(!strcmp(partita->arg[oggetto].enigma.risposta, buf)){    //risposta corretta, vetrina viene sbloccata
                partita->arg[oggetto].bloccato = false; 
                strcpy(buf, "\nLa vetrina si è aperta! Guarda bene cosa contiene\n");
            }else{
                strcpy(buf, "\nRisposta sbagliata :( \n");
            }
        }else{
            //vetrina già sbloccata
            strcpy(buf, "\nLa vetrina è già stata aperta. Guarda bene cosa contiene!\n"); 
        }
    }else if(!strcmp(obj, "chiave")){     //oggetto libero
        for(i=0; i< N_ZAINO; i++){
            if(!strcmp(partita->zaino[i], "chiave")){
                strcpy(buf, "--> Chiave già raccolta\n");
                break; 
            }
            if(!strcmp(partita->zaino[i], "\0")){
                strcpy(partita->zaino[i], obj); 
                partita->token++; 
                strcpy(buf, "--> Chiave messa nello zaino correttamente.Hai conquistato un token!\nProsegui l'avventura!\n"); 
                zaino++; 
                break; 
            }
        }
        if(i == N_ZAINO){     //zaino pieno, applico politica FIFO 
                //chiave sostituisce l'oggetto inserito per primo
                strcpy(partita->zaino[(zaino%4)], obj); 
                strcpy(buf, "--> Amuleti messi nello zaino correttamente.\nProsegui l'avventura!\n");
                zaino++; 
            }
    }else if(!strcmp(obj, "teca")){       //viene sbloccata solo se si possiedono i 3 token 
        if(partita->token == 3){
            strcpy(buf, partita->arg[oggetto].enigma.domanda);
            ret = my_send(sd, buf, "invio domanda teca"); 
            if(ret == -1){
                exit(1); 
            }

            memset(buf, 0, sizeof(buf));   
            //attendo risposta 
            ret = my_recv(sd, buf, "risposta codice teca");
            if(ret == -1){
                exit(1); 
            }

            if(!strcmp(partita->arg[oggetto].enigma.risposta, buf)){    //risposta corretta, giocatore vince 
                strcpy(buf, partita->arg[oggetto].descr);
                vittoria = true;     
            }else{
                strcpy(buf, "Risposta sbagliata :(\n");
            }
        }else{
            strcpy(buf, "--> Conquista tutti i token per poter accedere al sistema di allarme della teca\n");
        }
    }else{
        strcpy(buf, "\nComando take non disponibile su questo oggetto\n");
    }

    //dobbiamo inviare il contenuto del buffer riempito in base all'esito della take 
    //se buf inizia con EN il server invia enigma, attende risposta e solo dopo arriva qui e invia esito risposta 
    ret = my_send(sd, buf, "invio risposta cmd take"); 
    if(ret == -1){
        exit(1); 
    }

    if(!vittoria){  //se l'utente non ha vinto invio anche info partita, altrimenti no 
        invia_info_partita(partita, sd); 
    }

}

void comando_use(struct Partite* partita, char* arg1, char* arg2, int sd){

    //controllo argomenti già fatto sul client 
    int i;
    bool trovato = false; 
    int oggetto = trova_ogg(partita, arg2);     //cerco pergamena cosi da poterla sbloccare 

    memset(buf, 0, sizeof(buf));

    if(partita->arg[oggetto].bloccato == false){    //pergamena già sbloccata 
        strcpy(buf, "Comando use già utilizzato. Osserva attentamente la pergamena!\n");
        
    }else {

        //devo controllare che l'utente abbia gli amuleti nello zaino
        for(i=0; i< N_ZAINO; i++){
            if(!strcmp(partita->zaino[i], "amuleti")){
                trovato = true;     //amuleti presenti nello zaino
                break; 
            }
        }

        if(!trovato){
            strcpy(buf, "Non possiedi l'oggetto amuleti\n"); 
        }else {
            partita->arg[oggetto].bloccato = false;         //pergamena sbloccata
            partita->token++; 
            strcpy(buf, "Fantastico! Questi amuleti sono stati fondamentali. Hai conquistato un token\nAdesso puoi leggere finalmente il contenuto di questa pergamena\n");
        }
    }

    //invio al client esito della use
    ret = my_send(sd, buf, "invio risposta cmd use");
    if(ret == -1){
        exit(1);
    }
    invia_info_partita(partita, sd); 

}


void comando_obj(struct Partite* partita, int sd){

    int i;
    bool vuoto=true; 
    memset(buf, 0, sizeof(buf)); 

    for(i=0; i< N_ZAINO; i++){
        if(strcmp(partita->zaino[i], "\0")){    //se l'oggetto esiste si concatena nel buffer
            vuoto=false; 
            sprintf(buf+strlen(buf), "%s\n", partita->zaino[i]);
        }
    }
    
    if(vuoto){   //se vuoto è rimasto true, vuoldire che lo zaino è vuoto
        strcpy(buf, "Zaino ancora vuoto. Esplora il Museo e raccogli gli oggetti\n"); 
    }

    //invio al client il contenuto dello zaino
    ret = my_send(sd, buf, "invio risposta cmd obj");
    if(ret == -1){
        exit(1);
    }

    invia_info_partita(partita, sd); 

}


void comando_time(int sd_player, int sd_host, struct Partite* p){

    memset(buf, 0, sizeof(buf)); 
    int num1, num2, num3;   //numeri inviati dall'host
    int scelta;     //pacco scelto dal giocatore

    sprintf(buf, "Il giocatore %s sta per teminare il tempo. Ha bisogno del tuo aiuto!\n", p->user); 

    //invio messaggio aiuto all'host speciale
    ret = my_send(sd_host, buf, "richiesta aiuto"); 
    if(ret <= 0){
        exit(1);
    }

    memset(buf, 0, sizeof(buf)); 
    //attendo risposta dall'host speciale
    ret = my_recv(sd_host, buf, "risposta host"); 
    if(ret <= 0){
        exit(1); 
    }

    sscanf(buf, "%d %d %d", &num1, &num2, &num3);       //salvo i numeri inviati dall'utente speciale 

    //invio al giocatore i 3 pacchi, in base al numero che sceglie avrà tempo aggiuntivo relativo
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "Scegli il pacco che preferisci, digitando il numero corrispondente.\n1-Tebe\n2-Alessandria\n3-Menfi\n"); 
   
    printf("Socket player: %d\n", sd_player); 
    ret = my_send(sd_player, buf, "invio pacchi"); 
    if(ret <= 0){
        exit(1);
    }

    memset(buf, 0, sizeof(buf)); 
    //attendo risposta dal giocatore
    ret = my_recv(sd_player, buf, "scelta giocatore"); 
    if(ret <= 0){
        exit(1); 
    }

    sscanf(buf, "%d", &scelta);
    if(scelta == 1){
        scelta = num1; 
    }else if(scelta == 2){
        scelta = num2;

    }else if(scelta == 3){  
        scelta = num3;
    }

    memset(buf, 0, sizeof(buf)); 
    
    sprintf(buf, "%d", scelta);
    
    //invio al giocatore il risultato della sua scelta 
    ret = my_send(sd_player, buf, "invio ok time"); 
    if(ret <= 0){
        exit(1);
    }

    invia_info_partita(p, sd_player);

}



