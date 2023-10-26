#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <util.h>
#include <conn.h>
#include <list.h>
#include <pthread.h>
#include <sys/time.h>
/**
 * @file threadpool.h
 * @brief Interfaccia per il ThreadPool
 */
 

/**
 *  @struct threadpool
 *  @brief Rappresentazione dell'oggetto threadpool
 */
typedef struct threadpool_t {
    pthread_mutex_t  lock;          // mutua esclusione nell'accesso all'oggetto
    pthread_cond_t   cond_qEmpty;   // usata per notificare al worker thread 
    pthread_cond_t   cond_qFull;    // usata per notificare un worker thread 
    
    pthread_t *threads;         // pool di Th worker
    int numthreads;             // numero di thread (size dell'array threads)
    int numthreadsAttivi;       // numero di task attualmente in esecuzione 

    char **queue;             // coda interna per task pendenti
    int queue_size;           // massima size della coda, puo' essere anche -1 ad indicare che non si vogliono gestire task pendenti
    int queue_length;         // numero di task nella coda dei task pendenti
    int head, tail;           // riferimenti della coda
    
    struct timespec delay;
} threadpool_t;

// crea la struttura dati con cui si puo' gestire l'entita' threadpool
threadpool_t *createThreadPool(int nWorker, int size, int delay);

// interrompe tutti i thread e distrugge l'oggetto pool
void destroyThreadPool();

// implementazione del comporamento del Th worker 
void *workerpool_thread(void *threadpool); 

// estrae un task dalla coda
char* pop(threadpool_t *pool);
// inserisce un task nella coda 
int  push(threadpool_t *pool, char *data); // retval == 0 oppure -1 in caso di errore

// legge il contenuto del file il cui nome e' passato come argomento e ne calcola il risultato finale 
long calcola(char* path);

#endif /* THREADPOOL_H_ */

