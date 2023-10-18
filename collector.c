#include <collector.h>

#define COLLECTOR printf("[COLLECTOR]");

// ----------VARIABILI GLOBALI-------------------
int listenfd;
int connfd; //fd della socket creata per la comunicazione 

listaF orderedList = NULL;
// ----------END VARIABILI GLOBALI-------------------

//-----funzioni chiamate tramite atexit() ---------------
/*
The atexit() function registers the given function to be called at nor‐
mal process termination. Functions so registered are called in the reverse order
of their registration.
*/
void cancellaSocketFile(){  unlink(SOCKNAME);   } 
//chiude tramite file descriptor (listenfd)
void closeServerSocket(){
    int notused;
    SYSCALL_EXIT("close", notused, close(listenfd), "close", "");
}
//chiude tramite file descriptor (connfd)
void chiudiConnessione(){
    int notused;
    SYSCALL_EXIT("close", notused, close(connfd), "close", "");
}
//-----END funzioni chiamate tramite atexit() ---------------
 
// ---------------SERVER-------------------
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

    // COLLECTOR printf("Nuovo utente accettato dal collector\n");  
} //END openServerSocket


void leggi(){
    int msgDim = 0;
    long sommatoria  = 0;
    char* nomeFile   = NULL;
            
    while(msgDim != -1){
        //lettura dimensione tot del messaggio
        if(readn(connfd, &msgDim, sizeof(int)) == -1){
            perror("readn1 msgDim");
            exit(EXIT_FAILURE);
        }
        // COLLECTOR printf("ricevuto msgDim == %d\n", msgDim);
        
        if(msgDim > 0){ //indica la dimensione del messaggio ed esclude la stampa e la terminazione forzata
            
            //creazione array di supporto per inserire la stringa che mi verra' inviata 
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
            // COLLECTOR printf("ricevuto: '%s'\n", nomeFile);

            //lettura valore sommatoria (long)
            if(readn(connfd, &sommatoria, sizeof(long)) == -1){
                free(nomeFile);
                perror("readn3 sommatoria");
                exit(EXIT_FAILURE);
            }
            // fprintf(stdout, "inserisco nella lista ordinata %ld \t %s \n", sommatoria, nomeFile);
            inserisciOrdina(&orderedList, sommatoria, nomeFile);
            // stampaListaF(orderedList);
            free(nomeFile);
            nomeFile = NULL;
        }
        else{

            if(msgDim == -2){ //messaggio di stampa
                free(nomeFile);
                stampaListaF(orderedList);
            }
        }
    }//arrivato messaggio di terminazione '-1'

    free(nomeFile);
}
