#include <stdio.h>          /* stdin, stdout */
#include <string.h>         /* memset() */
#include <arpa/inet.h>      /* htons() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include <sys/select.h>     /* select(), FD_ZERO, FD_SET, and so on */
#include "common.h"

/* This function handles the client processing loop: It reads a line of
 * text from standard input, writes it to the server, reads back the
 * server's echo of the line, and outputs the echoed line to standard output
 *
 * We can now rewrite our str_cli() function, this time using select(), so
 * we are notified as soon as the server process terminates. The problem
 * with that earlier version was that we could be blocked in the call to
 * fgets() when something happened on the socket. Our new version blocks in
 * a call to select() instead, waiting for either standard input or the
 * socket to be readable.
 *      Figure 6.8. Conditions handled by select() in str_cli().
 *                          client
 *                  -------------------------
 *                  |                       | select() for readability
 *   data or EOF --># stdin                 | on either standard input
 *                  |        socket         | or socket
 *                  -----------#-------------
 *                           / | \
 *                    error /  |  \ EOF
 *                  ------/----|---\--------
 *              TCP |    ^     |    ^      | 
 *                  |    |     |    |      |
 *                  -----|-----|----|-------
 *                     RST   data  FIN
 * Three conditions are handled with the socket:
 * 1. If the peer TCP sends data, the socket becomes readable and read()
 * returns greater than 0 (i.e., the number of bytes of data).
 * 2. If the peer TCP sends a FIN (the peer process terminates), the socket
 * becomes readable and read() returns 0 (EOF).
 * 3. If the peer TCP sends an RST (the peer host has crashed and rebooted),
 * the socket becomes readable, read() returns -1, and errno contains the
 * specific error code.
 */
static void str_cli(FILE *fp, int sockfd)
{
    int maxfdp1;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];

    /* The loop terminates when fgets() returns a null pointer, which occurs
     * when it encounters either an end-of-file (EOF) or an error. Out
     * Fgets() wrapper function checks for an error and aborts if one
     * occurs, so Fgets() returns a null pointer only when an end-of-file
     * is encountered.
     */
    FD_ZERO(&rset);
    for ( ;; ) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        
        /* We only need one descriptor set--to check for readability. This
         * set is initialized by FD_ZERO and then two bits are turned on
         * using FD_SET: the bit corresponding to the standard I/O file
         * descriptor, fp, and the bit corresponding to the socket, sockfd.
         * The function fileno() converts a standard I/O file pointer into
         * its corresponding descriptor. select() (and poll()) work only
         * with descriptors.
         */
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
            if (Readline(sockfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            Fputs(recvline, stdout);
        }

        if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
            if (Fgets(sendline, MAXLINE, fp) == NULL)
                return;     /* all done */
            Writen(sockfd, sendline, strlen(sendline));
        }
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
