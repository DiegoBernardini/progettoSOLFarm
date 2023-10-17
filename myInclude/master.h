#if !defined(_MASTER_H)
#define _MASTER_H

#include <util.h>
#include <conn.h>
#include <list.h>
#include <threadpool.h>

#include <signal.h>
#include <pthread.h>
#include <time.h>

void openClientSocket();
void closeClientSocket();

int myConnect(int sockfd, const struct sockaddr* sa_server, socklen_t addrlen);

void pushList();
// void mascheraSegnali();

#endif /* _MASTER_H */

