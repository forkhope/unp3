#include <stdio.h>
#include <stdlib.h>     /* exit() */
#include <string.h>     /* memset() */
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>  /* inet_pton(), htons() */
#include "common.h"     /* err_sys(), err_quit() */

/* This is an implementation of a TCP time-of-day client. This client
 * establishes a TCP connection with a server and the server simply sends
 * back the current time and date in a human-readable format.
 *
 * This program is protocol-dependent on IPv4. We allocate and initialize
 * a sockaddr_in structure, we set the family of this structure to AF_INET,
 * and we specify the first argument to socket() as AF_INET.
 */
int main(int argc, char *argv[])
{
    int sockfd, n;
    char recvline[BUFSIZ];
    struct sockaddr_in servaddr;

    /* 书中练习1.4提到: Modify Figure 1.5 by placing a counter in the while
     * loop, counting the number of times read() returns a value greater
     * than 0. Print the value of the counter before terminating.
     */
    int counter;

    if (argc != 2) {
        err_quit("Usage: %s IP-address", argv[0]);
    }

    /* 1.创建一个socket
     * The socket() function creates an Internet (AF_INET) stream
     * (SOCK_STREAM) socket, which is a fancy name for a TCP socket. The
     * function returns a small integer descriptor that we can use to
     * identify the socket in all future function calls.
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket AF_INET SOCK_STREAM error");

    /* 2.填充 socket 地址结构体
     * We fill in an Internet socket address structure (a servaddr_inf
     * structure named servaddr) with the server's IP address and port
     * number. We set the entire structure to 0 using memset(), set the
     * address family to AF_INET, set the port number to 13 (which is the
     * well-known port of the daytime server on any TCP/IP host that
     * supports this service) and set the IP address to the value specified
     * as the first command-line argument(argv[1]). The IP address and port
     * number fields in this structure must be in specific formats: We call
     * the library function htons ("host to network short") to convert the
     * binary port number, and we call the library function inet_pton()
     * ("presentation to numeric") to convert the ASCII command-line
     * argument (for example, 192.168.1.88) into the proper format.
     */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(13);      /* daytime server */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        err_sys("inet_pton error for: %s\n", argv[1]);
    }

    /* 3.使用该 socket 发起 TCP 连接
     * The connect() function, when applied to a TCP socket, establishes a
     * TCP connection with the server specified by the socket address
     * structure pointed to by the second argument. We must also specify the
     * length of the socket address structure as the third argument to
     * connect(), and for Internet socket address structures, we always let
     * the compiler calculate the length using C's sizeof operator.
     */
    if (connect(sockfd, (struct sockaddr *)&servaddr,
                sizeof(servaddr)) < 0)
        err_sys("connect error");

    /* 4.建立连接后,就可以读写socket
     * We read the server's reply and display the result using the standard
     * I/O fputs() function. We must be careful when using TCP because it is
     * a byte-stream protocol with no record boundaries. We cannot assume
     * that the server's reply will be returned by a single read().
     * Therefore, when reading from a TCP socket, we always need to code the
     * read() in a loop and terminate the loop when either read() returns 0
     * (i.e., the other end closed the connection) or a value less than 0
     * (an error).          In this example, the end of the record is being
     * denoted by the server closing the connection.
     */
    counter = 0;
    while ((n = read(sockfd, recvline, BUFSIZ)) > 0) {
        ++counter;
        recvline[n] = '\0';     /* null terminate */
        if (fputs(recvline, stdout) == EOF)
            err_sys("fputs error");
    }
    if (n < 0)
        err_sys("read error");

    /* The value printed is always 1. */
    printf("counter = %d\n", counter);

    exit(0);
}
