#if !defined(_UTIL_H)
#define _UTIL_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>


#if !defined(BUFSIZE)
#define BUFSIZE 255
#endif

#if !defined(EXTRA_LEN_PRINT_ERROR)
#define EXTRA_LEN_PRINT_ERROR   512
#endif

#define DEBUG printf("DEBUG\n");

#define MALLOC(ptr, sz)			   	\
    if ((ptr=(char*)malloc(sz)) == NULL) { 	\
	perror("malloc");		   	\
	exit(EXIT_FAILURE);		   	\
    }

#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }


#define SYSCALL_PRINT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	errno = errno_copy;			\
    }

#define SYSCALL_RETURN(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	errno = errno_copy;			\
	return r;                               \
    }

#define CHECK_EQ_EXIT(name, X, val, str, ...)	\
    if ((X)==val) {				\
        perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define CHECK_NEQ_EXIT(name, X, val, str, ...)	\
    if ((X)!=val) {				\
        perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define ATEXIT(f) \
    if(atexit(f) !=0) { \
      fprintf(stderr, "problems with 'atexit(f)'\n");\
      exit(EXIT_FAILURE); \
    }


/**
 * \brief Procedura di utilita' per la stampa degli errori
 *
 */
static inline void print_error(const char * str, ...) {
    const char err[]="ERROR: ";
    va_list argp;
    char * p=(char *)malloc(strlen(str)+strlen(err)+EXTRA_LEN_PRINT_ERROR);
    if (!p) {
	perror("malloc");
        fprintf(stderr,"FATAL ERROR nella funzione 'print_error'\n");
        return;
    }
    strcpy(p,err);
    strcpy(p+strlen(err), str);
    va_start(argp, str);
    vfprintf(stderr, p, argp);
    va_end(argp);
    free(p);
}


/** 
 * \brief Controlla se la stringa passata come primo argomento e' un numero.
 * \return  0 ok  1 non e' un numero   2 overflow/underflow
 */
static inline int isNumber(const char* s, long* n) {
  if (s==NULL) return 1;
  if (strlen(s)==0) return 1;
  char* e = NULL;
  errno=0;
  long val = strtol(s, &e, 10);
  if (errno == ERANGE) return 2;    // overflow/underflow
  if (e != NULL && *e == (char)0) {
    *n = val;
    return 0;   // successo 
  }
  return 1;   // non e' un numero
}

#define LOCK(l)     if ((errno = pthread_mutex_lock(&l)) !=0 ){  \
    fprintf(stderr, "ERRORE FATALE lock\n");                      \
    exit(EXIT_FAILURE);                                           \
  }  

#define UNLOCK(l)   if ((errno = pthread_mutex_unlock(&l)) !=0 ){  \
  fprintf(stderr, "ERRORE FATALE unlock\n");                      \
  exit(EXIT_FAILURE);                                           \
}

#define WAIT(c, l)  if ((errno = pthread_cond_wait(&c, &l))!=0)       {      \
  fprintf(stderr, "ERRORE FATALE wait\n");          \
  exit(EXIT_FAILURE);                                           \
  }

#define SIGNAL(c)   if ((errno = pthread_cond_signal(&c)) !=0 ){  \
  fprintf(stderr, "ERRORE FATALE signal\n");                      \
  exit(EXIT_FAILURE);                                           \
}

#define BROADCAST(c) if ((errno = pthread_cond_broadcast(&c)) !=0 ){  \
  fprintf(stderr, "ERRORE FATALE broadcast\n");                      \
  exit(EXIT_FAILURE);                                           \
}  


#define LOCK_RETURN(l, r)  if (pthread_mutex_lock(&l)!=0)        { \
  fprintf(stderr, "ERRORE FATALE lock\n");        \
  return r;               \
}

#define UNLOCK_RETURN(l,r)    if (pthread_mutex_unlock(&l)!=0)      {  \
  fprintf(stderr, "ERRORE FATALE unlock\n");        \
  return r;               \
 } 

int nanosleep(const struct timespec *req, struct timespec *rem);

#endif /* _UTIL_H */
