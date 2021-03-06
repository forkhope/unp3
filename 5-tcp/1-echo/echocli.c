#include <stdio.h>          /* stdin, stdout */
#include <string.h>         /* memset() */
#include <arpa/inet.h>      /* htons() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include "common.h"

/* This function handles the client processing loop: It reads a line of
 * text from standard input, writes it to the server, reads back the
 * server's echo of the line, and outputs the echoed line to standard output
 */
static void str_cli(FILE *fp, int fd)
{
    char sendline[MAXLINE], recvline[MAXLINE];

    /* The loop terminates when fgets() returns a null pointer, which occurs
     * when it encounters either an end-of-file (EOF) or an error. Out
     * Fgets() wrapper function checks for an error and aborts if one
     * occurs, so Fgets() returns a null pointer only when an end-of-file
     * is encountered.
     */
    while (Fgets(sendline, 4, fp) != NULL) {
        Writen(fd, sendline, strlen(sendline));

        if (Readline(fd, recvline, MAXLINE) == 0)
            err_quit("str_cli: server terminated prematurely");

        Fputs(recvline, stdout);
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
