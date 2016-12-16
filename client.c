#include <fcntl.h>

#include "ftrans.h"

#define TTL_TIME 1

void printHelp(void);

int main(int argc, char *argv[]) {
    unsigned int currwin = 1;
    int sockfd = 0;
    char recvBuff[260];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr, resp_addr;
    struct timespec spec;
    unsigned int salen = sizeof(serv_addr);
    windows wins;

    if (argc < 4) {
        printHelp();
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("\n Error : Could not create socket \n");
        return 1;
    }

    memset((unsigned char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));  // port

    if (inet_aton(argv[2], &serv_addr.sin_addr) == 0) {
        perror("inet_aton() failed\n");
        return 1;
    }

    if (strlen(argv[1]) > 256) {
        printf("Filename too large");
        return 1;
    }

    FILE *fp;
    fp = fopen(argv[1], "rb");
    if (NULL == fp) {
        printf("Error opening file");
        return 1;
    }

    unsigned char *fips = calloc(sizeof(char), 260);
    unsigned int z;
    for (z = 0; z < 260 && z < strlen(argv[1]) + 1; z++) {
        fips[z] = argv[1][z];
    }

    // make socket non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // Send filename to server
    printf("Sending filename of size %d: \"%s\"\n", strlen(argv[1]), argv[1]);
    if (sendto(sockfd, fips, 260, 0, (struct sockaddr *)&serv_addr, salen) ==
        -1) {
        perror("Failed sendto\n");
        return 1;
    }

    wins.requests = NULL;

    addwindow(&wins, fips, 260);

    while (1) {
        unsigned char *buff = calloc(sizeof(unsigned char), 260);
        unsigned char *ackbuff = calloc(sizeof(unsigned char), 8);
        int bytesReceived = 0;

        // make sure all data is received
        do {
            window *reswins = chktimewindows(&wins, TTL_TIME);

            while (reswins != NULL) {
                reswins = (*reswins).next;
            }

            while (reswins != NULL) {
                if (sendto(sockfd, (*reswins).buff, (*reswins).size + 4, 0,
                           (struct sockaddr *)&serv_addr, salen) == -1) {
                    perror("Failed sendto\n");
                    return 1;
                } else {
                    clock_gettime(CLOCK_REALTIME, &spec);
                    (*reswins).ms = floor((spec.tv_nsec / 1.0e6) + 0.5);
                }
                reswins = (*reswins).next;
            }

            // read acks and remove related windows
            bytesReceived = (recvfrom(sockfd, ackbuff, 4, 0,
                                      (struct sockaddr *)&resp_addr, &salen));
            if (bytesReceived >= 4) {
                printf("Recieved response of size %d :%04x\n", bytesReceived, *((unsigned int *)ackbuff));
                removewindow(&wins, *((unsigned int *)ackbuff));
            } 

        } while (remainingwindows(wins) >= WINDOW_SIZE);

        int nread = fread(buff, 1, 256, fp);
        if (nread <= 0) {
            perror("Failed to read from file.");
            return 1;
        }

        memcpy(buff + nread, &currwin, sizeof(currwin));
        buff[nread] = ((char *)&currwin)[0];
        buff[nread + 1] = ((char *)&currwin)[1];
        buff[nread + 2] = ((char *)&currwin)[2];
        buff[nread + 3] = ((char *)&currwin)[3];

        currwin++;

        printf("Bytes read %d \n", nread);

        if (nread > 0) {
            printf("Sending \n");
            if (sendto(sockfd, buff, nread + 4, 0,
                       (struct sockaddr *)&serv_addr, salen) == -1) {
                perror("Failed sendto\n");
                return 1;
            }

            addwindow(&wins, buff, nread);
        }

        if (nread < 256) {
            if (feof(fp)) {
                printf("End of file\n");
            }

            if (ferror(fp)) {
                printf("Error reading\n");
                return 1;
            }

            // make sure all data is received
            while (remainingwindows(wins) > 0) {
                window *reswins = chktimewindows(&wins, TTL_TIME);

                printf("Verifying all windows are recieved");
                printf("Pending windows %d\n",remainingwindows(wins));
                printwindows(wins);

                while (reswins != NULL) {
                    printf("Stale window: %d\n", (*reswins).id);
                    reswins = (*reswins).next;
                }

                while (reswins != NULL) {
                    if (sendto(sockfd, (*reswins).buff, (*reswins).size, 0,
                               (struct sockaddr *)&serv_addr, salen) == -1) {
                        perror("Failed sendto\n");
                        return 1;
                    } else {
                        clock_gettime(CLOCK_REALTIME, &spec);
                        (*reswins).ms = floor((spec.tv_nsec / 1.0e6) + 0.5);
                    }
                    reswins = (*reswins).next;
                }

                bytesReceived =
                    (recvfrom(sockfd, ackbuff, 4, 0,
                              (struct sockaddr *)&resp_addr, &salen));
                if (bytesReceived >= 4) {
                    printf("Recieved response of size %d :%04x\n", bytesReceived, *((unsigned int *)ackbuff));
                    removewindow(&wins, *((unsigned int *)ackbuff));
                } 
            }

            close(sockfd);
            printf("File send completed\n");
            break;
        }
    }
    return 0;
}

void printHelp(void) { printf("USAGE: ./client <Filename> <IP> <Port>\n"); }
