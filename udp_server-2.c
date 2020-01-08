// complie using
//			gcc udp_server.c -o udp_server
//			gcc udp_client.c -o udp_client


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main() {

    time_t seconds;
    seconds = time(NULL);
    int ret;
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char recv_buffer[1024];
    char send_buffer[1024];
    int recv_len;
    socklen_t len;

    // This is a memory buffer to hold the message for retrieval.
    // I just declare a static character array for simplicity. You can
    // use a linked list or std::vector<...>. My array is only big enough
    // to hold one message, which is the most recent one.
    // Note that one more character is needed to hold the null-terminator
    // of a C string with a length of 200, i.e., strlen(msg) == 200).
    char recent_msg[201];
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // The servaddr is the address and port number that the server will
    // keep receiving from.
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(32000);

    cliaddr.sin_port = htons(32111);

    bind(sockfd,
         (struct sockaddr *) &servaddr,
         sizeof(servaddr));

    while (1) {
        FILE *outfile;
        outfile = fopen ("log.txt", "a");
        if (outfile == NULL)
        {
            fprintf(stderr, "\nError opend file\n");
            exit (1);
        }
        len = sizeof(cliaddr);
        recv_len = recvfrom(sockfd, // socket file descriptor
                            recv_buffer,       // receive buffer
                            sizeof(recv_buffer),  // max number of bytes to be received
                            0,
                            (struct sockaddr *) &cliaddr,  // client address
                            &len);             // length of client address structure

        if (recv_len <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }

        int m=0;
        memset(send_buffer, 0,sizeof(send_buffer));
        if (recv_buffer[0] != 0x44 || recv_buffer[1] != 0x59) {

            // Bad message!!! Skip this iteration.
            printf("Error: Unrecognized command format");
            continue;

        } else {

            if (recv_buffer[2] == 0x01) {

                // Note that you need to erase the memory to store the most
                // recent message first. C string is always terminated by
                // a '\0', but when we send the line, we did not send
                // this null-terminator.
                memset(recent_msg, 0, sizeof(recent_msg));

                // Note that recv_buffer[3] contains the length of the text
                // line, see the protocol description.
                // Be careful, you may need to do some sanity checks on
                // recv_buffer[3], i.e., whether it is non-zero, etc.

                memcpy(recent_msg, recv_buffer + 4, recv_buffer[3]);
                fprintf (outfile, "<%ld>[%s:%d] post#%s",
                        seconds,
                        inet_ntoa(cliaddr.sin_addr),
                        ntohs(cliaddr.sin_port),
                        recent_msg);

                char sus[]="post_ack#successful";

                fprintf (outfile, "<%ld>[%s:%d] %s\n",
                        seconds,
                        inet_ntoa(servaddr.sin_addr),
                        ntohs(servaddr.sin_port),
                        sus);

                m=strlen(sus);
                memcpy(send_buffer + 4,sus , m);
                send_buffer[0] = 0x44; // These are constants you defined.
                send_buffer[1] = 0x59;
                send_buffer[2] = 0x02;
                send_buffer[3] = m;

            } else if (recv_buffer[2] == 0x03) {

                fprintf (outfile, "<%ld>[%s:%d] retrieve#\n",
                        seconds,
                        inet_ntoa(cliaddr.sin_addr),
                        ntohs(cliaddr.sin_port));

                char str[]="retrieve_ack#";
                strcat(str,recent_msg);
                m=strlen(str);

                fprintf (outfile, "<%ld>[%s:%d] %s",
                        seconds,
                        inet_ntoa(servaddr.sin_addr),
                        ntohs(servaddr.sin_port),
                        str);

                memcpy(send_buffer + 4,str , m-1);
                send_buffer[0] = 0x44; // These are constants you defined.
                send_buffer[1] = 0x59;
                send_buffer[2] = 0x04;
                send_buffer[3] = m;

            } else {
                printf("Error: Unrecognized command format");
                // Wrong message format. Skip this iteration.
                continue;
            }

        }

        sendto(sockfd,                   // the socket file descriptor
               send_buffer,                    // the sending buffer
               m+4, // the number of bytes you want to send
               0,
               (struct sockaddr *) &cliaddr, // destination address
               sizeof(cliaddr));

        fclose (outfile);
    }

    return 0;
}

