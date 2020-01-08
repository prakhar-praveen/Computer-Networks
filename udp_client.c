#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{

    int ret;
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char send_buffer[1024];
    char user_input[1024];
    char recv_buffer[1024];
    int MAGIC_1, MAGIC_2, OPCODE_POST, OPCODE_RETRIEVE;
    

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // The "servaddr" is the server's address and port number,
    // i.e, the destination address if the client needs to send something.
    // Note that this "servaddr" must match with the address in the
    // UDP server code.
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(32000);

    // TODO: You may declare a local address here.
    // You may also need to bind the socket to a local address and a port
    // number so that you can receive the acks from the socket.
    // You may also skip the binding process. In this case, every time you
    // call sendto(), the source port may be different.

    // Optionally, you can call connect() to bind the socket to a
    // destination address and port number. Since UDP is connection less,
    // the connect() only set up parameters of the socket, no actual
    // datagram is sent. After that, you can call send() and recv() instead
    // of sendto() and recvfrom(). However, people usually do not do this
    // for a UDP based application layer protocol.

    while (1)
    {

        // The fgets() function read a line from the keyboard (i.e, stdin)
        // to the "send_buffer".
        fgets(user_input,
              sizeof(user_input),
              stdin);

        // m is a variable that temporarily holds the length of the text
        // line typed by the user (not counting the "post#" or "retrieve#".
        int m = 0;
        
        // Compare the first five characters, check input format.
        // Note that strncmp() is case sensitive.
        if (strncmp(user_input, "post#", 5) == 0)
        {
        
            // Now we know it is a post message that should be sent.
            // Extract the input text line length, and copy the line to
            // the payload part of the message in the send_buffer. Note
            // that the first four bytes are the header, so when we
            // copy the input line of text to the destination memory
            // buffer, i.e., the send_buffer + 4, there is an offset of
            // four bytes after the memory buffer that holds the whole
            // message.

            // Note that in C and C++, array and pointer are interchangable.

            // TODO: Check the user input format and make sure it is not
            // empty or longer than 200 characters.

           
            m = strlen(user_input) - 5;
            memcpy(send_buffer + 4, user_input + 5, m);
            if(m>0 && m<200)
            {
            send_buffer[0] = MAGIC_1; // These are constants you defined.
            send_buffer[1] = MAGIC_2;
            send_buffer[2] = OPCODE_POST;
            send_buffer[3] = m;
            }
            else
            {
                printf("Error: Unerecognized command format\n"); continue;
            }
        }
           
        else if (strncmp(user_input, "retrieve#",9) == 0)
        {
                m = strlen(user_input) - 9;
                memcpy(send_buffer + 4, user_input + 5, m);
            // TODO: Check whether it matches to "retrieve#".
            // Note that a retrieve message has no payload, i.e., m = 1.
                if(m==1)
                {
                send_buffer[0] = MAGIC_1; // These are constants you defined.
                send_buffer[1] = MAGIC_2;
                send_buffer[2] = OPCODE_RETRIEVE;
                send_buffer[3] = m;
                }
                else
                {
                    printf("Error: Unerecognized command format\n"); continue;

                }
        }

        else
        {

            // If it does not match any known command, just skip this
            // iteration and print out an error message.
            printf("Error: Unrecognized command format");
             continue;

        }


        // The sendto() function send the designated number of bytes in the
        // "send_buffer" to the destination address.
        ret = sendto(sockfd,                   // the socket file descriptor
               send_buffer,                    // the sending buffer
               m + 4, // the number of bytes you want to send
               0,
               (struct sockaddr *) &servaddr, // destination address
               sizeof(servaddr));             // size of the address

        if (ret <= 0)
        {
            printf("sendto() error: %s.\n", strerror(errno));
            return -1;
        }
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int len = sizeof(cliaddr);

        // TODO: You are supposed to call the recvfrom() function here.
        // The client will receive the acknowledgement from the server.
        recvfrom(sockfd, // socket file descriptor
        recv_buffer,       // receive buffer
        sizeof(recv_buffer),  // max number of bytes to be received
        0,
        (struct sockaddr *) &cliaddr,  // client address
                 &len);
        fflush(stdin);
        printf("%s\n",recv_buffer+4);
    }
    close(sockfd);
    return 0;
}
