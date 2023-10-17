/**
 * @file threadpool.c
 * @brief File di implementazione dell'interfaccia Threadpool
 */
#include <threadpool.h>
#define DEBUG printf("DEBUG\n");


//flag dichiarati in master.c
int terminaCodaFlag;
int stampaFlag;

int clientSocket;
pthread_mutex_t lockSocket;

threadpool_t *pool = NULL;

//ok
threadpool_t *createThreadPool(int nthreads, int qSize, int delay) {
    //TO DO si puo' eliminare questo controllo? lo faccio gia' quando leggo gli argomenti? 
    if(nthreads <= 0 || qSize <= 0) {
        errno = EINVAL;
        return NULL;
    }
    
    //TO DO MALLOC(threadpool_t *pool, sizeof(threadpool_t) );
    // threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if (pool == NULL) return NULL;

    // condizioni iniziali 
    pool->numthreads        = 0;
    pool->taskonthefly      = 0;
    pool->queue_size        = qSize; //TODO qSize-1 
    pool->head = pool->tail = 0;

    pool->queue_length = 0; //aggiunto

    pool->delay.tv_sec  = delay / 1000;
    pool->delay.tv_nsec = (delay % 1000) *1000000;



    /* Allocate thread and task queue */
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
    if (pool->threads == NULL) {
       free(pool);
       return NULL; //-1
    }

    pool->queue = (char**)malloc(sizeof(char*) *(qSize));
    if (pool->queue == NULL) {
        free(pool);
        free(pool->threads);
        return NULL; //-1
    }

    for(int i = 0; i < pool->queue_size; i++){
     pool->queue[i] = NULL;
    }

    if ((errno = pthread_mutex_init(&(pool->lock), NULL)) != 0){
        free(pool);
        free(pool->threads);
        free(pool->queue);
        return NULL; //-1
    }

  
    if ((errno = pthread_cond_init(&(pool->cond_qFull), NULL) != 0)){
        free(pool);
        free(pool->threads);
        free(pool->queue);
        if((errno = pthread_mutex_destroy(&(pool->lock))) != 0){ perror("destroying mutex in createThreadPool");  }
        return NULL; //-1
    }


    if ((errno = pthread_cond_init(&(pool->cond_qEmpty), NULL)) != 0){
        free(pool);
        free(pool->threads);
        free(pool->queue);
        if((errno = pthread_mutex_destroy(&(pool->lock))) != 0){        perror("destroying mutex in createThreadPool");  }
        if((errno = pthread_cond_destroy(&(pool->cond_qFull))) != 0){   perror("destroying cond_qFull in createThreadPool");    }
        return NULL; //-1
    }

    // //creazione pool di worker
    for(int i = 0; i < nthreads; i++) {
        if((errno = pthread_create(&(pool->threads[i]), NULL, workerpool_thread, (void*)pool)) != 0) {
         // errore fatale, libero tutto forzando l'uscita dei threads 
            terminaCodaFlag = 1; 
            destroyThreadPool();

            // errno = EFAULT;
            return NULL;
        }
    }

    pool->numthreads   = nthreads;
    pool->taskonthefly = nthreads; //numero di thread attivi
    
    // printf("pool->numthreads = %d, pool->taskonthefly = %d\n", pool->numthreads , pool->taskonthefly );
    atexit(destroyThreadPool);
    
    return pool;
}//END createThreadPool

// ok
void destroyThreadPool() {
    if(pool == NULL) {
        errno = EINVAL;
        perror("destroyThreadPool");
        return;
    }
    //free threads[i]
    if(pool->threads) free(pool->threads);
    
    //free queue
    if(pool-> queue != NULL){
        for (int i = 0; i < pool->queue_size; i++){
            if(pool->queue[i] != NULL){
                free(pool->queue[i]);
            }
        }
        free(pool->queue);
    }

    if ((errno = pthread_cond_destroy (&(pool->cond_qFull)))  != 0) perror("destroy cond_qFull");
    if ((errno = pthread_cond_destroy (&(pool->cond_qEmpty))) != 0) perror("destroy cond_qEmpty");
    if ((errno = pthread_mutex_destroy(&(pool->lock)))        != 0) perror("destroy mutex");
    
    free(pool);  

    // printf("Pool correttamente liberato\n");  
}//END destroyThreadPool


/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief funzione eseguita dal thread worker che appartiene al pool
 */
void *workerpool_thread(void *arg) {    
    threadpool_t *myPool = (threadpool_t *)arg; // cast
    char *path = NULL;
    long sommatoria;
    int msgDim;
    int notused;
    // int myid = 30;

    //-----utilizzata per la fase di test 
    // pthread_t self = pthread_self();
    // int myid = -1;
    // while(myid < 0){
    //     for (int i=0;i<myPool->numthreads;++i)
    //     if (pthread_equal(myPool->threads[i], self)) {
    //         myid = i;
    //         printf("Creato thread n.%d\n", myid);
    //         break;
    //     }
    // } 
    //--------------------

    while(1){
        LOCK(myPool->lock); //prendo la lock


        //attendo di ricevere un messaggio oppure il flag di terminazione  
        while(myPool->queue_length == 0 && terminaCodaFlag == 0){
            //coda vuota 
            // printf("thread %d bloccato sulla WAIT perche' la coda e' vuota\n", myid);
            WAIT(myPool->cond_qEmpty, myPool->lock);//aspetto
        }

         // printf("terminaCodaFlag = %d queue_length =  %d\n", terminaCodaFlag, myPool->queue_length );

        if(myPool->queue_length == 0 && terminaCodaFlag == 1){
            //coda vuota, siamo in fase di terminazione, devo uscire
            //l'ultimo thread in vita manda il messaggio di terminazione
            // printf("myPool->taskonthefly = %d\n", myPool->taskonthefly);
            if(myPool->taskonthefly == 1){
                msgDim = -1;
                LOCK(lockSocket);
                // printf("[THREADPOOL]INVIO AL COLLECTOR SEGNALE DI TERMINAZIONE\n");
                // Invio al collector '-1'
                SYSCALL_EXIT("writen terminaCodaFlag", notused, writen(clientSocket, &msgDim, sizeof(int)), "write terminaCodaFlag" , "");
                UNLOCK(lockSocket);

                // destroyThreadPool();
            }

            myPool->taskonthefly--;
            //se invece e' colpa del signal handler
            SIGNAL(myPool->cond_qFull);
            // SIGNAL(myPool->cond_qEmpty);

            UNLOCK(myPool->lock)

            // printf("thread %d mi sto per distruggere, restano taskonthefly = %d \n", myid, myPool-> taskonthefly);

            return NULL; // pthread exit
        }

        // messaggio di stampa 
        if(stampaFlag == 1){
            UNLOCK(myPool->lock);

            msgDim = -2;
            LOCK(lockSocket);
            // CHECK_EQ_EXIT("writen stampaFlag", writen(clientSocket, &msgDim, sizeof(int)), -1, "writen stampaFlag");
            SYSCALL_EXIT("writen stampaFlag", notused, writen(clientSocket, &msgDim, sizeof(int)), "write stampaFlag" , "");

            UNLOCK(lockSocket);
            stampaFlag = 0;//ripristino

        }
        else{
            //nuovo task
            CHECK_EQ_EXIT("pop", path = pop(myPool), NULL, "pop", "");

            //faccio la signal al master
            SIGNAL(myPool->cond_qFull); //segnalo che la coda non e'
            UNLOCK(myPool->lock);       //rilascio la lock

            //calcolo il risultato
            CHECK_EQ_EXIT("calcola", sommatoria = calcola(path), -1, "calcola", "");

            // printf("[WORKER ] sommatoria calcolata sul file '%s' = '%ld'\n", path, sommatoria);
            // printf("[WORKER '%d'] calcola\n", myid);
            
            //calcolo la lunghezza del messaggio 
            msgDim = strlen(path);
            msgDim++ ; // '\0' 

            //invio la risposta
            LOCK(lockSocket);
            // CHECK_EQ_EXIT("writen - msgDim", writen(clientSocket, &msgDim, sizeof(int)),      -1, "writen - msgDim");
            SYSCALL_EXIT("writen msgDim", notused, writen(clientSocket, &msgDim, sizeof(int)), "write msgDim" , "");
            // CHECK_EQ_EXIT("writen - path",   writen(clientSocket, path, msgDim),              -1, "writen - path");
            SYSCALL_EXIT("writen path", notused, writen(clientSocket, path, msgDim), "write path" , "");
            // CHECK_EQ_EXIT("writen - value",  writen(clientSocket, &sommatoria, sizeof(long)), -1, "writen - value");
            SYSCALL_EXIT("writen value", notused, writen(clientSocket, &sommatoria, sizeof(long)), "write value" , "");

            UNLOCK(lockSocket);

            free(path);
            // path = NULL;
        }

    }
    UNLOCK(myPool->lock);

    return NULL; //pthread exit
}


//----------AGGIUNTA--------------------
/*
 funzione che inserisce l'elemento "data" nella coda concorrente, gestisce l'acquisizione della lock
 retval == 0 oppure -1 in caso di errore
*/
int push(threadpool_t *threadpool, char* data){
    if(threadpool == NULL) {
        errno = EINVAL;
        return -1;
    }
    //prendo la lock 
    LOCK_RETURN(threadpool->lock, -1);

    // printf("sto pushando '%s' dim queue '%d'\n", data, threadpool->queue_length);
    while(threadpool->queue_length == threadpool->queue_size && terminaCodaFlag != 1){
        // coda piena
        // printf("la push si e' bloccata sulla WAIT perche' la coda e' piena \n");
        WAIT(threadpool->cond_qFull, threadpool->lock);//aspetto
    }

    //aggiorno i puntatori 
       if(terminaCodaFlag != 1){
        threadpool->queue[threadpool->tail] = data;
        threadpool->tail = (threadpool->tail+1) % threadpool-> queue_size;
        threadpool->queue_length++;
    }

    SIGNAL(threadpool->cond_qEmpty); //segnalo che la coda non e' vuota 
    UNLOCK_RETURN(threadpool->lock,-1); // rilascio la lock 

    return 0;
}

/*
funzione che si occupa di estrarre un elemento dalla coda concorrente,
NON si occupa della gestione della lock, 
e' una funzione che deve essere eseguita quando si e' gia' in possesso della lock
retval== data oppure NULL in caso di errore 
*/
char* pop(threadpool_t *threadpool){
    if(threadpool == NULL) {
        errno = EINVAL;
        return NULL;
    }
    char *data = threadpool ->queue[threadpool->head];
    threadpool->queue[threadpool->head] = NULL;
    threadpool->head = (threadpool->head+1) % threadpool-> queue_size;
    threadpool->queue_length--;

    return data;
}

//ok
long calcola(char* filePath){
    long sum = 0, i = 0, dato;
    FILE *fp = NULL;

    if((fp = fopen(filePath, "rb")) == NULL ){
        fprintf(stderr, "aprendo il file %s\n", filePath );
        return -1;
    }

    // printf("i numeri sono :");
    rewind(fp);

    while(fread(&dato, sizeof(long), 1, fp) == 1){
        // printf(" %ld ", dato);
        sum += (dato*i);
        i++;
    }

    //EOF raggiunto
    if(feof(fp)) {
        fclose(fp);
        return sum;
    }

    //errore sconosciuto 
    fclose(fp);
    return -1;
}