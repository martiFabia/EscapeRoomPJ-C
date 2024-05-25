#include "utility.h"

int ret; 


//gestisco l'invio dei messaggi: invio prima la lunghezza, poi il messaggio
int my_send(int sd, char* buf, char*err){
    int len;
    uint16_t lmsg;
	len = strlen(buf)+1;
	lmsg = htons(len);

    ret=send(sd, (void*)&lmsg, sizeof(uint16_t), 0);
	if(ret == -1){
		fprintf(stderr, "Errore invio lunghezza mess (%s): %s\n", err, strerror(errno));
		return ret; 
	}
    ret=send(sd, (void*)buf, len, 0);
    if(ret==-1){
       fprintf(stderr, "Errore invio mess (%s): %s\n", err, strerror(errno));
        return ret; 
    }

    return ret; 
}


//gestisco ricezione dei messaggi: prima ricevo lunghezza, poi il messaggio
int my_recv(int sd, char* buf, char* err){
    int len;
	uint16_t lmsg;

    ret=recv(sd, (void*)&lmsg, sizeof(uint16_t), 0);
	if(ret==-1){
        fprintf(stderr, "Errore ricezione lunghezza mess (%s): %s\n", err, strerror(errno));
        return ret; 
    }

	len = ntohs(lmsg);
    ret=recv(sd, (void*)buf, len, 0);
    if(ret==-1){
       fprintf(stderr, "Errore ricezione mess (%s): %s\n", err, strerror(errno));
        return ret; 
    }

    return ret;
}

