#include <collector.h>

#define COLLECTOR printf("[COLLECTOR]");

// ----------VARIABILI GLOBALI-------------------
int listenfd;
int connfd; //nuovo fd per la nuova socket per la comunicazione appena creata

listaF orderedList = NULL;
// ----------END VARIABILI GLOBALI-------------------

//-----funzioni chiamate tramite atexit() ---------------
/*
The atexit() function registers the given function to be called at nor‐
mal process termination. Functions so registered are called in the reverse order
of their registration.
*/
void cancellaSocketFile(){  
    unlink(SOCKNAME);
    // COLLECTOR printf("socketFile eliminato lato collector\n");
    // printf("COLLECTOR -> CHIUSOOOOOOOOOOO\n");

} 
//chiude tramite file descriptor
void closeServerSocket(){
    int notused;
    SYSCALL_EXIT("close", notused, close(listenfd), "close", "");//chiudo la listen socket
    // COLLECTOR printf("Server non piu' in ascolto\n");
}

//chiude tramite file descriptor
void chiudiConnessione(){
    int notused;
    SYSCALL_EXIT("close", notused, close(connfd), "close", "");//chiudo la listen socket
    // COLLECTOR printf("Comunicazione interrotta con il client\n");

}
//-----END funzioni chiamate tramite atexit() ---------------
 
// ---------------SERVER-------------------
//ok
void openServerSocket (){
    //creo la socket
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
    atexit(closeServerSocket);
    
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;    
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused;
    //assegno l'indirizzo alla socket
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "bind", "");
    
    // COLLECTOR printf("Server in ascolto\n"); 
    
    atexit(cancellaSocketFile);

    //setto la socket in modalità passiva e definisco un numero di massimo di connessioni pendenti
    SYSCALL_EXIT("listen", notused, listen(listenfd, MAXBACKLOG), "listen", "");
    //bloccante

    SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL ,NULL), "accept","");
    
    atexit(chiudiConnessione); 
    // --------END-----------------------------
    // COLLECTOR printf("Nuovo utente accettato dal collector\n");  
} //END openServerSocket

//sembra ok 
void leggi(){
    int msgDim = 0;
    long sommatoria  = 0;
    char* nomeFile   = NULL;
            
    // if((nomeFile = (char*) malloc(sizeof(char)*BUFSIZE)) == NULL){
    //     perror("malloc nomeFile");
    //     exit(EXIT_FAILURE);
    // }
    // char *buf        = NULL;//delete
    // msg_t* messaggio = NULL;//delete

    while(msgDim != -1){
        // COLLECTOR printf("terminaCodaFlag = %d", terminaCodaFlag);
    // do{
        // printf("sono dentro al ciclo DO di leggi()");
        // messaggio = (msg_t*) malloc(sizeof(msg_t));//TO DO CHECK 
        // if(!messaggio){
        //     perror("malloc messaggio");
        //     exit(EXIT_FAILURE);
        // } 

        //lettura dimensione tot del messaggio
        if(readn(connfd, &msgDim, sizeof(int)) == -1){
            // free(messaggio);
            perror("readn1");
            exit(EXIT_FAILURE);
        }
        
        // COLLECTOR printf("ricevuto msgDim == %d\n", msgDim);
        
        if(msgDim > 0){ //indica la dimensione del messaggio ed esclude la stampa e la terminazione forzata
            // printf("caso msgDim >0\n");
            
            // messaggio = (msg_t*) malloc(sizeof(msg_t));//TO DO CHECK 
            
            // if(!messaggio){
            //     perror("malloc messaggio");
            //     exit(EXIT_FAILURE);
            // }


            // //alloco buffer di supporto
            // if((buf = (char*) malloc(sizeof(char) * msgDim)) == NULL){
            //     free(messaggio);
            //     perror("malloc buffer");
            //     exit(EXIT_FAILURE);
            // }
              //alloco buffer di supporto


            //lettura nome file
            // if(readn(connfd, buf, msgDim) == -1){
            //     free(messaggio);
            //     free(buf);
            //     perror("readn2");
            //     exit(EXIT_FAILURE);
            // }

            if((nomeFile = (char*) malloc(sizeof(char)*BUFSIZE)) == NULL){
                perror("malloc fileName");
                exit(EXIT_FAILURE);
            }
            
            //lettura nome file
            if(readn(connfd, nomeFile, msgDim) == -1){
                free(nomeFile);
                perror("readn2 nomeFile");
                exit(EXIT_FAILURE);
            }
            

            // if((messaggio->fileName = (char*) malloc(sizeof(char) * msgDim)) == NULL){
            //     free(messaggio);
            //     free(buf);
            //     perror("malloc fileName");
            //     exit(EXIT_FAILURE);
            // }
            
            //copio il nome del file nel mio array di supporto
            // strncpy(messaggio->fileName, buf, msgDim);
            // free(buf);
            // buf = NULL;

            // COLLECTOR printf("ricevuto: '%s'\n", messaggio->fileName);

            //lettura valore sommatoria (long)
            // if(readn(connfd, &sommatoria, sizeof(long)) == -1){
            //     free(messaggio->fileName);
            //     free(messaggio);
            //     perror("readn3");
            //     exit(EXIT_FAILURE);
            // }

            if(readn(connfd, &sommatoria, sizeof(long)) == -1){
                free(nomeFile);
                perror("readn3 sommatoria");
                exit(EXIT_FAILURE);
            }
            // messaggio->value = sommatoria;

            // printf("inserisco nella lista ordinata %ld \t %s \n", sommatoria, nomeFile);
            // inserisciOrdina(&orderedList, sommatoria, messaggio->fileName);
            inserisciOrdina(&orderedList, sommatoria, nomeFile);
            // stampaListaF(orderedList);
            free(nomeFile);
            nomeFile = NULL;
        }
        else{
            // printf("ramo else\n");
            if(msgDim == -2){ //messaggio di stampa
                printf("caso msgDim = -2\n");
                // free(messaggio);
                free(nomeFile);
                stampaListaF(orderedList);
            }
        }
    // } while(msgDim != -1); //messaggio di terminazione '-1'
    }//messaggio di terminazione '-1'
    // COLLECTOR printf("ho ricevuto il messaggio di terminazione -1\n");
    // sleep(1);
    // free(messaggio);
    free(nomeFile);
}
