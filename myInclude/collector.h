#if !defined(COLLECTOR_H)
#define COLLECTOR_H

#include <util.h>
#include <conn.h>
#include <list.h>

typedef struct message{
	long value;
	char *fileName; 
} msg_t;

void openServerSocket();

void closeServerSocket();	// close 
void cancellaSocketFile();  // unlink
void chiudiConnessione();	// close

void leggi();

#endif /* COLLECTOR_H */