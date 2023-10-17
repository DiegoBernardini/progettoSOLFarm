#include <master.h>
// segnali
#include <time.h>
#define MASTER printf("[MASTER] ");


// -----VARIABILI GLOBALI----------

int clientSocket;//clientSocket = fd che puo' essere utilizzato per I/O
pthread_mutex_t lockSocket = PTHREAD_MUTEX_INITIALIZER;

int terminaCodaFlag    = 0;//segnala di non inserire altro in coda 
int terminaGestoreFlag = 0;//segnala al master che il gestore e' stato terminato da un segnale e non e' necessara la kill 
int stampaFlag         = 0;//segnala ai thread di lanciare un messaggio di stampa

//threadpool.c
threadpool_t *pool;
// -END VARIABILI GLOBALI----------

//ok
void closeClientSocket(){
    int notused;
    SYSCALL_EXIT("close", notused, close(clientSocket), "close", "");
    // MASTER printf("Client non piu connesso\n");
}

//ok
void openClientSocket(){
    //stabilisco la connessione tramite una socket 
    int notused;
    SYSCALL_EXIT("socket", clientSocket, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
    atexit(closeClientSocket);

    struct sockaddr_un sa_server;
    memset(&sa_server, '0', sizeof(sa_server));
    sa_server.sun_family = AF_UNIX;    
    strncpy(sa_server.sun_path,SOCKNAME, strlen(SOCKNAME)+1);

    SYSCALL_EXIT("connect", notused, myConnect(clientSocket, (struct sockaddr*)&sa_server, sizeof(sa_server)), "connect", "");
    //connessione stabilita

	// MASTER printf("connesso al collector\n"); 
}//END openClientConnection


/*
    EAGAIN          Resource temporarily unavailable       
    ECONNREFUSED    No-one listening on the remote address.
    EINTR           The system call was interrupted by a signal that was caught; see signal(7).
    ENOENT          No such file or directory (POSIX.1) (errore non fatale)
*/
int myConnect(int clientSocket, const struct sockaddr* sa_server, socklen_t addrlen){
    int attempt = 3;

    while((errno = 0), (connect(clientSocket, sa_server, addrlen)) == -1){
    	if(errno ==  ENOENT || errno == EAGAIN || errno == EINTR || (errno == ECONNREFUSED && (attempt--) != 0)){
    		// attempt--;
    		// printf("'connect' failed, %d tentativi rimanenti, errno=%d\n", attempt, errno);
    		sleep(1);
    	}
    	else return -1;
    }
    return 0;
}//END myConnect


void pushList(lista *listaFileBinari){
    lista tmp = NULL;
    
    while (*listaFileBinari != NULL && terminaCodaFlag == 0){

        if (push(pool, (*listaFileBinari)->path) == -1){ //pusho un file 
            perror("PUSH");
            exit(EXIT_FAILURE);
        }
        
        nanosleep(&(pool->delay), NULL); //aspetto
        // sleep(1);

        //elimino dalla lista il file appena processato 
        tmp = (*listaFileBinari)->next;
        free(*listaFileBinari);
        //vado avanti
        *listaFileBinari = tmp;
    }
}//END pushList