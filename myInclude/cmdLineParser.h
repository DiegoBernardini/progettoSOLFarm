#if !defined(_CMDLINEPARSER_H)
#define _CMDLINEPARSER_H

#include <util.h>
#include <list.h>

#include <getopt.h>
#include <dirent.h>


#if !defined(LUNGHEZZA_STRINGHE)
#define LUNGHEZZA_STRINGHE 	256
#endif

#if !defined(DEFAULT_NTHREAD)
#define DEFAULT_NTHREAD 	4
#endif

#if !defined(DEFAULT_QUEUE_LEN)
#define DEFAULT_QUEUE_LEN 	8
#endif

#if !defined(DEFAULT_TIME_DELAY)
#define DEFAULT_TIME_DELAY 	0
#endif

#if !defined(OPTION_LIST)
#define OPTION_LIST 		"n:q:d:t:"
#endif

//FLAG DI SISTEMA 
typedef struct {
  int n; //#worker
  int q; //queue length 
  int t; //delta time
  lista l; 
} flag;

void printOptionList(char* s);

int isdot(const char dir[]);
char* cwd() ;
void exploreDir(const char* , lista* );
int cmdParse(int, char**, flag* );

#endif /* _CMDLINEPARSER_H */

