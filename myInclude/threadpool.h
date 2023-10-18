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
    int taskonthefly;           // numero di task attualmente in esecuzione 

    char **queue;             // coda interna per task pendenti
    int queue_size;           // massima size della coda, puo' essere anche -1 ad indicare che non si vogliono gestire task pendenti
    int queue_length;         // numero di task nella coda dei task pendenti
    int head, tail;           // riferimenti della coda
    
    struct timespec delay;
} threadpool_t;


threadpool_t *createThreadPool(int nWorker, int size, int delay);

/**
 * @function destroyThreadPool
 * @brief stoppa tutti i thread e distrugge l'oggetto pool
 * @param pool  oggetto da liberare
 * @param force se 1 forza l'uscita immediatamente di tutti i thread e libera subito le risorse, se 0 aspetta che i thread finiscano tutti e soli i lavori pendenti (non accetta altri lavori).
 *
 * @return 0 in caso di successo <0 in caso di fallimento ed errno viene settato opportunamente
 */
// void destroyThreadPool(threadpool_t *pool /*int force*/);
void destroyThreadPool();

void *workerpool_thread(void *threadpool); 


char* pop(threadpool_t *pool);
int  push(threadpool_t *pool, char *data); // retval == 0 oppure -1 in caso di errore

long calcola(char* path);

#endif /* THREADPOOL_H_ */

