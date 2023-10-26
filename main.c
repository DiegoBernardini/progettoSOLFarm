#define _POSIX_C_SOURCE  200112L

#include <util.h>
#include <conn.h>
#include <list.h>

#include <cmdLineParser.h>
#include <collector.h>
#include <master.h>
#include <threadpool.h>

// segnali
#include <signal.h>
//waitpid
#include <sys/wait.h>

// -----VARIABILI GLOBALI----------
//master.c
int endProtocolFlag;	//segnala di non inserire altro in coda 
int endSighandlerFlag;	//segnala al master che il gestore e' stato terminato da un segnale e non e' necessaria la kill 
int stampaFlag;			//segnala ai thread di inviare un messaggio di stampa al collector

sigset_t mask;//maschera dei segnali

//collector.c
listaF orderedList;

//threadpool.c
threadpool_t *pool;

// int clientSocket;			//variabili utilizzate per inviare messaggi di prova tra processi 
// pthread_mutex_t lockSocket;	//variabili utilizzate per inviare messaggi di prova tra processi 

//------FUNZIONI------------------

// funzione utilizzata per testare se la connessione tra i due processi tramite socket e' stata stabilita correttamente
// void inviaMessaggioDiProva();

void mascheraSegnali();
 
void *sigHandlerF(void *arg);// funzione eseguita dal signal handler thread


//-----funzioni chiamate tramite atexit() ---------------
/*
The atexit() function registers the given function to be called at nor‐
mal process termination. Functions so registered are called in the reverse order
of their registration.
*/ 
void shutDown(){	unlink(SOCKNAME);	}

void freeListaF(){	deleteListaF(&orderedList);	}
//-----END funzioni chiamate tramite atexit() ---------------

//------MAIN-----------------------

int main(int argc, char* argv[]){
	
	//1 controllo quanti argomenti sono arrivati 
	if (argc == 1){
		printOptionList(argv[0]);
		exit(EXIT_FAILURE);
	}

	//2 maschero i segnali
	mascheraSegnali();

	//3 creo il secondo processo (collector)
	pid_t pid;
	SYSCALL_EXIT("fork", pid, fork(), "fork", "");
	if(pid == 0){// Collector -> Figlio (simulerà il Server)

		openServerSocket(); //apro socket lato servere
		
		atexit(freeListaF);

		runCollector();

		stampaListaF(orderedList); 
	} 
	else {// MasterWorker -> Padre (simulera' il client)
		atexit(shutDown);
	//-------------MASTERWORKER ------------
	
	// 1-> Creazione thread che gestisce i segnali 
	pthread_t sighandler_thread;
	CHECK_NEQ_EXIT("pthread_create", pthread_create(&sighandler_thread, NULL, sigHandlerF, (void*)&mask), 0, "pthread_create", "");

    // 2 -> Parsing cmd  
	flag flagSystem = { DEFAULT_NTHREAD,
					DEFAULT_QUEUE_LEN, 
					DEFAULT_TIME_DELAY, 
					NULL };// flag.l e' la lista dei file 

	CHECK_EQ_EXIT("cmdParse", cmdParse(argc, argv, &flagSystem), -1, "cmdParse","");

    // ------operazioni di stampa per test -------------------
	// MASTER printf("num workers 'n'=%d\nlength 'q'=%d\ntime 't'=%d\n", flagSystem.n, flagSystem.q, flagSystem.t);
	// MASTER printf("***Stampo lista***\n");
	// stampaLista(flagSystem.l);
	// deleteLista(&flagSystem.l);
	// printf("cancello lista\n\n");
	// stampaLista(flagSystem.l);
	//--------------------------------------------------------

	// 3 -> Creazione threadPool 
	CHECK_EQ_EXIT("createThreadPool", pool = createThreadPool(flagSystem.n, flagSystem.q, flagSystem.t), NULL, "createThreadPool", "");

	// 4 -> apro connessione tramite socket 
 	openClientSocket();

	// A QUESTO PUNTO E' TUTTO SETTATO CORRETTAMENTE 

 	// inviaMessaggioDiProva();

	// inserisco nella coda concorrente la lista dei file ottenuti dal parsing (flagSystem.l)
	pushList(&flagSystem.l);
 	
 	//SE VA TUTTO BENE E NON ARRIVANO SEGNALI, INIZIO IL PROTOCOLLO DI TERMINAZIONE 
 	
 	//ottengo lock 
	LOCK(pool->lock);
	// avvio protocollo di terminazione 
	endProtocolFlag = 1;
	//e lo comunico a tutti i thread
	BROADCAST(pool->cond_qEmpty);
	// sblocco lock
	UNLOCK(pool->lock);

	//attendo (join thread)
	for (int i = 0; i < flagSystem.n; ++i){
		if(pthread_join(pool-> threads[i], NULL) != 0){
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
	}

	int notused;
	SYSCALL_EXIT("waitpid", notused, waitpid(pid, NULL, 0), "waitpid", "");

	//mi occupo anche del signal handler
	if(endSighandlerFlag == 0){//invio io il segnale 
		CHECK_NEQ_EXIT("pthread_kill", pthread_kill(sighandler_thread, SIGTERM), 0, "pthread_kill", "");
	}

	CHECK_NEQ_EXIT("join sighandler", pthread_join(sighandler_thread, NULL), 0, "join sighandler", "");
	
	}//END else 
	return 0;
}

//------------IMPLEMENTAZIONI FUNZIONI------------
/*
* funzione con cui vado a mascherare i segnali
* in questo programma verra' ereditata da entrambi i processi    
*/
void mascheraSegnali(){
    struct sigaction s;
    memset(&s, 0, sizeof(s)); // inizializzo
    s.sa_handler = SIG_IGN;	  // registro gestore
    int notused;  
    //installo gestore per SIGPIPE per evitare di essere terminato da una scrittura su un socket 
    SYSCALL_EXIT("sigaction", notused, sigaction(SIGPIPE, &s, NULL), "sigaction", "");

  	//inizializzo la maschera
    SYSCALL_EXIT("sigemptyset", notused, sigemptyset(&mask), "sigemptyset", "");
    
    //setto a 1 la posizione in mask corrispondente al segnale che voglio gestire
    SYSCALL_EXIT("sigaddset", notused, sigaddset(&mask, SIGHUP) , "sigaddset", ""); 
    SYSCALL_EXIT("sigaddset", notused, sigaddset(&mask, SIGINT) , "sigaddset", "");
    SYSCALL_EXIT("sigaddset", notused, sigaddset(&mask, SIGQUIT), "sigaddset", "");
    SYSCALL_EXIT("sigaddset", notused, sigaddset(&mask, SIGTERM), "sigaddset", "");
    SYSCALL_EXIT("sigaddset", notused, sigaddset(&mask, SIGUSR1), "sigaddset", "");

    CHECK_NEQ_EXIT("pthread_sigmask", pthread_sigmask(SIG_BLOCK, &mask,NULL), 0, "pthread_sigmask", ""); 
    // SIG_BLOCK = or di mask e la vecchia signal mask 
}


/* 
* funzione eseguita dal signal handler thread: permette di attendere in modo sincrono tramite sigwait.
* In questo modo non installo e utilizzo handler limitanti dal punto di vista di utilizzo di funzioni e accesso a variabili globali  
* per poter usare correttamente sigwait i segnali che verranno attesi devono essere stati mascherati 
*/ 
void *sigHandlerF(void *arg) {
    sigset_t *set = (sigset_t*)arg;

	int True = 1, sig;

	while(True){
		CHECK_NEQ_EXIT("sigwait", sigwait(set, &sig), 0, "sigwait", "");
		switch(sig) {
			case SIGHUP: //1
			case SIGINT: //2
			case SIGQUIT://3
			case SIGTERM://15
			// il processo deve completare i propri task eventualmente presenti nella coda dei task da elaborare,
			// non leggendo piu' eventuali altri file in input
				endProtocolFlag	  = 1;
			    endSighandlerFlag = 1;
			    // printf("[Segnale]ESCO e' arrivato il segnale %d\n", sig);
			    pthread_exit(NULL);	    	    
		    case SIGUSR1://10
		    // il processo MasterWorker notifica al processo Collector di stampare i risultati ricevuti fino a quel momento
			    stampaFlag = 1;
   			    // printf("[Segnale] e' arrivato il segnale %d\n", sig);
			    break; 
			default: ; 
		}
    }	   
   return NULL;	   
}

/*
void inviaMessaggioDiProva(){  
	char* path1 = "messaggio di prova1";
	char* path2 = "messaggio di prova2";
	// char* path3 = "messaggio di prova3";
	int notused;
	long sommatoria = 30;
	int msgDim = strlen(path1);
    msgDim= msgDim+1; // '\0'
    
    LOCK(lockSocket);

    SYSCALL_EXIT("writen msgDim", notused, writen(clientSocket, &msgDim, sizeof(int)), "write msgDim" , "");
    // printf("scritto %d sulla socket\n", msgDim);
    SYSCALL_EXIT("writen path", notused, writen(clientSocket, path1, msgDim), "write path" , "");
    // printf("scritto %s sulla socket\n", path1);
	SYSCALL_EXIT("writen long", notused, writen(clientSocket, &sommatoria, sizeof(long)), "write long" , "");
    // printf("scritto %ld sulla socket\n", sommatoria);

	msgDim = strlen(path2);
    sommatoria = 40;

    SYSCALL_EXIT("writen msgDim", notused, writen(clientSocket, &msgDim, sizeof(int)), "write msgDim" , "");
    // printf("scritto %d sulla socket\n", msgDim);
    SYSCALL_EXIT("writen path", notused, writen(clientSocket, path2, msgDim), "write path" , "");
    // printf("scritto %s sulla socket\n", path2);
	SYSCALL_EXIT("writen long", notused, writen(clientSocket, &sommatoria, sizeof(long)), "write long" , "");
    // printf("scritto %ld sulla socket\n", sommatoria);

    msgDim = -1;

    SYSCALL_EXIT("writen msgDim", notused, writen(clientSocket, &msgDim, sizeof(int)), "write msgDim" , "");
    // printf("scritto %d sulla socket\n", msgDim);
    UNLOCK(lockSocket); 
}
*/