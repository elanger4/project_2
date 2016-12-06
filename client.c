#include "ftrans.h"

void printHelp(void);

int main(int argc, char *argv[]) {
    int sockfd = 0;
    char recvBuff[256];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;
    windows wins;

    if(argc < 2) {
        printHelp();
        return 1;
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5550);  // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    FILE *fp;
    fp = fopen(argv[1], "rb");
    if (NULL == fp) {
        printf("Error opening file");
        return 1;
    }

    // Send filename to server
    write(sockfd, argv[1], strlen(argv[1]));

    wins.requests = NULL;

    while (1) {
        char *buff = calloc(sizeof(unsigned char), 256);
        int nread = fread(buff, 1, 256, fp);
        unsigned int currwin = 0;
        printf("Bytes read %d \n", nread);

        if (nread > 0) {
            printf("Sending \n");
            write(sockfd, buff, nread);
            printf("Added window:%d\n", currwin = addwindow(&wins, buff));
            removewindow(&wins, currwin);
            printf("Removed window\n");
        }

        int * ack = NULL;
        int bytesReceived = read(sockfd, ack, 4);

        if (bytesReceived < 0) {
            perror("Error Message");
            return 1;
        }
        printf("recived bytes: %d\n", bytesReceived);
        printf("Recieved ack for buf #: %d", *(ack));
        /*
        if ((read(sockfd, ack, 4)) > 0) {
            printf("Recieved ack for buf #: %d", *(ack));
        }
        */

        if (nread < 256) {
            if (feof(fp)) {
                printf("End of file\n");
            }

            if (ferror(fp)) {
                printf("Error reading\n");
            }

            // make sure all data is received
            while (remainingwindows(&wins) > 0) {
                chktimewindows(&wins, 99);
            }

            close(sockfd);
            printf("File send completed\n");
            break;
        }
    }

    return 0;
}

void printHelp(void) {
    printf("USAGE: ./client <filename>\n");
}
