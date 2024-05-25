#include "partite.h"
#include "utility.h"


//allocazione dinamica di una nuova partita iniziata
struct Partite* crea_partita(char* u){
    int i; 
    struct Partite* new_p; 
    new_p = (struct Partite*) malloc(sizeof(struct Partite)); 
    strcpy(new_p->user, u);
    new_p->scenario = 0;
    new_p->token = 0;
  
    for(i=0; i<3; i++){
        strcpy(new_p->zaino[i], "\0");      //zaino vuoto 
    }
    return new_p;
}


void inizializza_arg(struct Partite* p_cur, char* s){

    // Inizializzazione degli oggetti/location e degli enigmi associati

    strcpy(p_cur->arg[0].nome, "amuleti"); 
    p_cur->arg[0].bloccato = true; 
    strcpy(p_cur->arg[0].enigma.domanda, "EN Quale fu la causa della morte di Cleopatra?\nA-si fece mordere da un serpente velenoso\nB-venne assassinata dal marito\nC-venne avvelenata dall'amante\nDigita la lettera della risposta corretta\n");
    strcpy(p_cur->arg[0].enigma.risposta, "A\n"); 
    strcpy(p_cur->arg[0].descr_blocc, "Questi amuleti catturano la tua attenzione, nel buio della stanza appaiono lucenti e luminosi.\nSembrano essere dedicati al dio del Sole Ra.\n\n");
    strcpy(p_cur->arg[0].descr, "Questi amuleti catturano la tua attenzione, nel buio della stanza appaiono lucenti e luminosi.\nSembrano essere dedicati al dio del Sole Ra.\n\n");

    strcpy(p_cur->arg[1].nome, "porta1"); 
    p_cur->arg[1].bloccato = false; 
    strcpy(p_cur->arg[1].descr, "\n****************SALA DEI PAPIRI******************\nQui antichi papiri coperti da geroglifici raccontano storie epiche dell'antico Egitto. Alcuni di essi sono esposti in una ++vetrina++.\nSu un tavolo vedi una **pergamena** srotolata.\n\n"); 

    strcpy(p_cur->arg[2].nome, "porta2"); 
    p_cur->arg[2].bloccato = true; 
    strcpy(p_cur->arg[2].enigma.domanda, "EN Porta bloccata. Scopri le seguenti parole:\n_ar_o_e \nt_ _ _n_ha_ _ _ \nM_ _ f _ \n(NB: scrivi le 3 parole su una riga separate da spazio)\n\n");
    strcpy(p_cur->arg[2].enigma.risposta, "faraone tutankhamon menfi\n");
    strcpy(p_cur->arg[2].descr_blocc, "Questa porta sembra essere ben protetta.\nSolo pochi eletti potranno accedervi ricostruendo antiche parole\n\n");
    strcpy(p_cur->arg[2].descr, "\n**************STANZA DEI GIOIELLI REALI*************\nUno spazio dedicato ai tesori reali, con corone, collane e gioielli indossati dai faraoni e dalle regine.\nLe pietre preziose brillano sotto luci accuratamente posizionate, creando una scena di magnificenza regale.\nAl centro della stanza prende la scena la famosa corona d’oro ornata di pietre preziose appartenuta al faraone Ramses II.\nMa attenzione, la magia svanisce subito appena vedi che è protetta da una **teca** di vetro blindata.\n\n");


    strcpy(p_cur->arg[3].nome, "vetrina"); 
    p_cur->arg[3].bloccato = true; 
    strcpy(p_cur->arg[3].enigma.domanda, "EN Guardati bene intorno!\nInserisci il codice a 4 cifre del lucchetto:\n\n");
    strcpy(p_cur->arg[3].enigma.risposta, "1208\n"); 
    strcpy(p_cur->arg[3].descr_blocc, "Questa vetrina sembra contenere oggetti preziosi. Un lucchetto la tiene ben chiusa.\n\n"); 
    strcpy(p_cur->arg[3].descr, "La vetrina presenta molteplici oggetti, ciascuno racconta un pezzo di storia dell’Anitco Egitto. Attirano la tua attenzione una pietra coperta da geroglifici, dei preziosi amuleti e una **chiave** d’oro.\n\n");

    strcpy(p_cur->arg[4].nome, "pergamena"); 
    p_cur->arg[4].bloccato = true; 
    strcpy(p_cur->arg[4].descr_blocc, "Nella stanza è molto buio, non riesci a leggere il suo contenuto.\n");
    strcpy(p_cur->arg[4].descr, "Il messaggio con il tempo è sbiadito ma per fortuna si riesce a leggere ancora qualcosa.\n-->La morte di Cleopatra, ultima sovrana dell'Egitto tolemaico, avvenne il 12 Agosto del 30 a.C. ad Alessandria d'Egitto, alla fine della guerra civile tra Ottaviano Augusto e Marco Antonio.\nSecondo la leggenda più famosa, la regina si sarebbe suicidata facendosi mordere da un serpente velenoso, precisamente un cobra egiziano\n\n");


    strcpy(p_cur->arg[5].nome, "chiave"); 
    p_cur->arg[5].bloccato = false; 
    strcpy(p_cur->arg[5].descr, "Questa chiave è fatta di puro oro e ricoperta di pietre preziose.\nUn gioiello davvero unico!\n\n"); 

    strcpy(p_cur->arg[6].nome, "teca"); 
    p_cur->arg[6].bloccato = true; 
    strcpy(p_cur->arg[6].enigma.domanda, "EN Complimenti hai raccolto tutti i token!\nMa non è ancora conclusa la missione.\nContinua la sequenza per trovare il codice segreto che sblocca il sistema di allarme:\n3,8,15,24 ,__, __\n(Inserisci i numeri mancanti senza spazi tra loro)\n\n");
    strcpy(p_cur->arg[6].enigma.risposta, "3548\n"); 
    strcpy(p_cur->arg[6].descr_blocc, "Questa teca contiene l'oggetto con maggior valore del Museo, è ben protetta!\n\n"); 
    strcpy(p_cur->arg[6].descr, "WIN Missione compiuta. Lupin è fiero di te!\n"); 

}

//funzione che invia le informazioni principali sulla partita in corso, chiamata dopo aver gestito ogni comando 
void invia_info_partita(struct Partite* partita, int sd){

    char buf[1024]; 
    int ret; 
    memset(buf, 0, sizeof(buf));  

    sprintf(buf, "Token raccolti: %d  Token mancanti: %d ", partita->token, 3-(partita->token));

    //invio al client le info partita, eccetto il tempo che è gestito direttamente dal client
    ret = my_send(sd, buf, "invio info partita");
    if(ret == -1){
        exit(1);
    }
}





