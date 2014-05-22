#include <stdio.h>          /* stdin, stdout */
#include <string.h>         /* memset() */
#include <arpa/inet.h>      /* htons() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include "common.h"
#include "sum.h"

/* This function handles the client processing loop: It reads a line of
 * text from standard input, writes it to the server, reads back the
 * server's echo of the line, and outputs the echoed line to standard output
 */
static void str_cli(FILE *fp, int fd)
{
    char sendline[MAXLINE];
    struct args args;
    struct result result;

    /* We now modify our client and server to pass binary values across the
     * socket, instead of text strings. We will see that this does not work
     * when the client and server are run on hosts with different byte
     * orders, or on hosts that do not agree on the size of a long integer
     * (例如,32位机器上,long int型是32位,64位机器上,long int型是64位).
     * 书中提到: If we run the client and server on two machines of the
     * same architecture, say two SPARC machines, everything works fine.
     * But when the client and server are on two machines of different
     * architectures (say the server is on the big-endian SPARC system
     * freebad and the client is on the little endian Intel system linux),
     * it does not work. The problem is that the two binary integers are
     * sent across the socket in little-endian format by the client, but
     * interpreted as big-endian integers by the server. We see that it
     * appears to work for positive integers but fails for negative
     * integers. There are really three potential problems with this example
     * 1.Different implementations store binary numbers in different formats
     * 2.Different implementations can store the same C datatype differently
     * For example, most 32-bit Unix systems use 32 bits for a long but
     * 64-bit systems typically use 64 bits for the same datatype. There is
     * no guarantee that a short, int, or long is of any certain size.
     * 3.Different implementations pack structures differently, depending on
     * the number of bits used for the various datatypes and the alignment
     * restrictions of the machine. Thereforce, it is never wise to send
     * binary structures across a socket.
     *
     * There are two common solutions to this data format problem:
     * 1. Pass all numeric data as text strings. This assumes that both
     * hosts have the same character set.
     * 2. Explicitly define the binary formats of the supported datatypes
     * (number of bits, big- or little-endian) and pass all data between
     * the client and server in this format.
     */
    while (Fgets(sendline, MAXLINE, fp) != NULL) {
        /* sscanf() converts the two arguments from text strings to binary,
         * and we call writen() to send the structure to the server.
         */
        if ((sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2)) != 2) {
            printf("invalid input: %s", sendline);
            continue;
        }
        Writen(fd, &args, sizeof(args));

        if (Readn(fd, &result, sizeof(result)) == 0)
            err_quit("str_cli: server terminated prematurely");

        printf("%ld\n", result.sum);
    }
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in srvaddr;

    if (argc != 2)
        err_quit("usage: echocli <IPaddress>");

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &srvaddr.sin_addr);

    Connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

    str_cli(stdin, sockfd);     /* do it all */

    return 0;
}
