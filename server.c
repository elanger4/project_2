#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    int numrv;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5550);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
        printf("Failed to listen\n");
        return -1;
    }

    while (1) {
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        unsigned char buff[256] = {0};
        int bytesReceived = 0;
        FILE *fp;

        if ((read(connfd, buff, 256)) > 0) {
            fp = fopen(buff, "wb");
            if (fp == NULL) {
                printf("File open error");
                close(connfd);
            }
        }

        while ((bytesReceived = read(connfd, buff, 260)) > 0) {
            printf("Bytes received %d\n", bytesReceived);
            fwrite(buff, 1, bytesReceived, fp);
            printf("here\n");
            write(connfd, buff + bytesReceived - 4, 4); // Sending ack to client
            printf("here\n");
        }

        printf("File transfer complete.");

        fflush(fp);
        fclose(fp);
        /*
                while(1)
                {
                    unsigned char buff[256]={0};
                    int nread = fread(buff,1,256,fp);
                    printf("Bytes read %d \n", nread);

                    if(nread > 0)
                    {
                        printf("Sending \n");
                        write(connfd, buff, nread);
                    }

                    if (nread < 256)
                    {
                        if (feof(fp))
                            printf("End of file\n");
                        if (ferror(fp))
                            printf("Error reading\n");
                        break;
                    }


                }
        */
        close(connfd);
    }

    return 0;
}
