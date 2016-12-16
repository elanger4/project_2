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

#define WINDOW_SIZE 20

typedef struct window {
    unsigned char *buff;
    long ms;
    unsigned int id;
    unsigned int size;
    struct window *next;
} window;

typedef struct windows {
    window *requests;
    unsigned int numRequests;
} windows;

unsigned int addwindow(windows *ws, unsigned char *buff, unsigned int size);
unsigned int addwindowindex(windows *ws, unsigned char *buff, unsigned int size,
                            unsigned int index);
int removewindow(windows *ws, unsigned int index);
window *getwindow(windows *ws, unsigned int index);
window *chktimewindows(windows *ws, long ttl);
unsigned int remainingwindows(windows ws);
void printwindows(windows ws);

#endif
