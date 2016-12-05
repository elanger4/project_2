#include "ftrans.h"

int main(void) {
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[256];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;
    windows wins;
    const char filename[] = "sample_file.txt";

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
    fp = fopen(filename, "rb");
    if (NULL == fp) {
        printf("Error opening file");
        return 1;
    }

    // Send filename to server
    write(sockfd, filename, strlen(filename));

    wins.requests = NULL;

    while (1) {
        unsigned char *buff = calloc(sizeof(unsigned char), 256);
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

