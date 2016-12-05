#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

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
window* getwindow(windows *ws,unsigned int index);
window* chktimewindows(windows *ws, long ttl);
unsigned int remainingwindows(windows *ws);

int main(void)
{
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[256];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;
    windows wins;
    const char filename[] = "sample_file.txt";

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5550); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    FILE *fp;
    fp = fopen(filename, "rb"); 
    if(NULL == fp) {
        printf("Error opening file");
        return 1;
    }

    //Send filename to server
    write(sockfd, filename, strlen(filename));

    wins.requests = NULL;

    while(1) {
        unsigned char *buff = calloc(sizeof(unsigned char),256);
        int nread = fread(buff,1,256,fp);
        unsigned int currwin = 0;
        printf("Bytes read %d \n", nread);        

        if(nread > 0) {
            printf("Sending \n");
            write(sockfd, buff, nread);
            printf("Added window:%d\n",currwin = addwindow(&wins,buff));
            removewindow(&wins,currwin);
            printf("Removed window\n");
        }

        if (nread < 256) {
            if (feof(fp)) {
                printf("End of file\n");
            }

            if (ferror(fp)) {
                printf("Error reading\n");
            }

            //make sure all data is received
            while(remainingwindows(&wins) > 0) {
                chktimewindows(&wins,99);
            }

            close(sockfd);
            printf("File send completed\n");
            break;
        }
    }

    return 0;
}

unsigned int addwindow(windows *ws, char *buff) {
    static unsigned int index = 0;
    struct timespec spec;
    window *iter = (*ws).requests; 
    window *tmp = calloc(sizeof(window),1);

    clock_gettime(CLOCK_REALTIME, &spec);
    (*tmp).ms = floor((spec.tv_nsec / 1.0e6)+0.5);
    (*tmp).id = index++;
    (*tmp).buff = buff;

    if((*ws).requests == NULL) {
        (*ws).requests = tmp;
    }
    else {
        while ((*iter).next != NULL) {
            iter = (*iter).next;
        }
        (*iter).next = tmp;    
    }

    return index-1;
}

int removewindow(windows *ws, unsigned int index) {
    window *tmp = (*ws).requests;

    while(tmp != NULL) {
        if((*tmp).id == index) {
            free(tmp);
            return 1;
        } else {
            tmp = (*tmp).next;
        }
    }
    return -1;
}

window* getwindow(windows *ws,unsigned int index) {
    window *tmp = (*ws).requests;

    while(tmp != NULL) {
        if((*tmp).id == index) {
            return tmp;
        } else {
            tmp = (*tmp).next;
        }
    }
    return NULL;
}

unsigned int remainingwindows(windows *ws) {
    unsigned int rem = 0;
    window *tmp = (*ws).requests;

    while(tmp != NULL) {
        tmp = (*tmp).next;
        rem++;
    }
    return rem;
}

window* chktimewindows(windows *ws, long ttl) {
    window *stale = NULL;
    window *iter = (*ws).requests;
    window *prev = NULL;
    long ms;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    ms = floor((spec.tv_nsec / 1.0e6)+0.5);
  
    while(iter != NULL) {
        if((*iter).ms + ttl > ms) {
            //we need to resend this message because the response has timed out
            // so we remove it and add it to the linked list of windows returned
            if(prev != NULL) {
                (*prev).next = (*iter).next;
                (*iter).next = stale;
                stale = iter;
            } else {
                (*ws).requests = NULL;
            }
        } else {
            prev = iter;
        }
        iter = (*iter).next;
    }
    return stale;
}
