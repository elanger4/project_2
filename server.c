#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ftrans.h"

#define PORT_NUM 5556
#define MAX_FILESIZE 9999999

int main(void) {
    int listenfd = 0;
    struct sockaddr_in serv_addr, cli_addr;
    unsigned int salen = sizeof(cli_addr);

    listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUM);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1) {
        unsigned char *buff = calloc(sizeof(char),260);
        int bytesReceived = 0;
        unsigned int fin = 1;
        FILE *fp;
        windows wins;

        wins.requests = NULL;

        while (fin && (bytesReceived = (recvfrom(listenfd, buff, 260,0,(struct sockaddr *)&cli_addr,&salen))) >= 0) {
            printf("Bytes received %d\n", bytesReceived);
            if(bytesReceived < 4) {
                printf("Ignoring packet of lenght <4\n");
            } else {
                printf("Packet Index: %d\n",*((unsigned int*)(buff+bytesReceived - 4)));
                addwindowindex(&wins,buff,bytesReceived-4,*((int*)(buff + bytesReceived - 4)));
                printf("Sending ack response\n");
                if (sendto(listenfd, buff + bytesReceived - 4, 4, 0, (struct sockaddr*) &cli_addr, salen) == -1)
                {
                    perror("sendto failed");
                    return 1;
                }
                //THIS WILL NEED TO BE REPLACED WITH A BETTER METHOD FOR DETECTING THE END OF THE TRANSMISSION
                //handle this by sending the files size after the filename
                if((bytesReceived-4) < 256) {
                    fin = 0;
                } else {
                    buff = calloc(sizeof(char),260);
                }
            }
        }

        //first and second are the file name and filesize respectively
        fp = fopen( (char *)(*(getwindow(&wins,0))).buff, "wb");
        if (fp == NULL) {
            printf("Invalid filename: %s",buff);
            perror("File open error");
            return 1;
        }

        removewindow(&wins,0);

        unsigned long findex = 1;
        while( (wins.requests != NULL) && (findex < MAX_FILESIZE) ) {
            window *tmp = getwindow(&wins,findex);
            if(tmp != NULL) {
                fwrite((*tmp).buff, 1, (*tmp).size, fp);
                removewindow(&wins,findex);
            }
            findex++;
        }

        printf("File transfer complete.\n");

        fflush(fp);
        fclose(fp);
    }

    return 0;
}
