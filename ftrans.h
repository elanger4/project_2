#ifndef _FTRANS_H_
#define _FTRANS_H_

#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define NUM_WINDOWS 20

typedef struct window {
    char *buff;
    long ms;
    unsigned int id;
    struct window *next;
} window;

typedef struct windows {
    window *requests;
    unsigned int numRequests;
} windows;

unsigned int addwindow(windows *ws, char *buff);
int removewindow(windows *ws, unsigned int index);
window *getwindow(windows *ws, unsigned int index);
window *chktimewindows(windows *ws, long ttl);
unsigned int remainingwindows(windows *ws);

#endif
