#include <stdlib.h>         /* exit() */
#include <string.h>         /* memset() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include <arpa/inet.h>      /* htonl(), htons() */
#include "common.h"

/* We can rewrite the server as a single process that uses select() to
 * handle any number of clients, instead of forking one child per client.
 *
 * Denial-of-Service Attacks:
 * Unfortunately, there is a problem with the server that we just showed.
 * Consider what happens if a malicious client connects to the server, sends
 * one byte of data (other than a newline), and then goes to sleep. The
 * server will call read(), which will read the single byte of data from the
 * client and block in the next call to read(), waiting for more data from
 * this client. The server is then blocked ("hung" may be a better term) by
 * this one client and will not service any other clients (either now client
 * connections or existing client's data) until the malicious client either
 * sends a newline or terminates.
 *
 * The basic concept here is that when a server is handling multiple
 * clients, the server can never block in a function call related to a
 * single client. Doing so can hang the server and deny service to all other
 * clients. This is called a denial-of-service attack. It does something to
 * the server that prevents it from servicing other legitimate clients.
 * Possible solutions are to: (i) use nonblocking I/O, (ii) have each client
 * serviced by a seperate thread of control (e.g., either spawn a process or
 * a thread to service each client), or (iii) place a timeout on the I/O
 * operations.
 */
int main(void)
{
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char line[MAXLINE];
    struct sockaddr_in srvaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

    Listen(listenfd, LISTENQ);

    maxi = -1;              /* index into client[] array */
    for (i = 0; i < FD_SETSIZE; ++i)
        client[i] = -1;     /* -1 indicates available entry */

    maxfd = listenfd;       /* initialize */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ;; ) {
        /* 由于 select() 函数会修改传入的 fd_set 参数,所以使用allset为rset
         * 赋值,保持allset不变,以供下次调用select()函数时,保持要监听的
         * fd_set 集合不变.
         * select() waits for something to happen: either the establishment
         * of a new client connection or the arrival of data, a FIN, or an
         * RST on an existing connection.
         */
        rset = allset;      /* strucutre assignment */
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {    /* new client connection */
            /* If the listening socket is readable, a new connection has
             * been established. We call accept() and update our data
             * structures accordingly. We use the first unused entry in
             * the client[] array to record the connected socket. The
             * number of ready descriptors is decremented, and if it is 0,
             * we can avoid the next for loop. This lets us use the return
             * value from select() to avoid checking descriptors that are
             * not ready.
             */
            connfd = Accept(listenfd, NULL, NULL);

            for (i = 0; i < FD_SETSIZE; ++i) {
                if (client[i] < 0) {
                    client[i] = connfd;     /* save descriptor */
                    break;
                }
            }
            if (i == FD_SETSIZE)
                err_quit("too many clients");

            FD_SET(connfd, &allset);    /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;         /* for select */
            if (i > maxi)
                maxi = i;               /* max index in client[] array */

            if (--nready <= 0)
                continue;       /* no more readable descriptors */
        }

        /* A test is made for each existing client connection as to whether
         * or not its descriptor is in the descriptor set returned by
         * select(). If so, a line is read from the client and echoed back
         * to the client. If the client closes the connection, read()
         * returns 0 and we update our data structures accordingly.
         */
        for (i = 0; i <= maxi; ++i) {   /* check all clients for data */
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &rset)) {
                if ((n = Readline(sockfd, line, MAXLINE)) == 0) {
                    /* connection closed by client. We never decrement the
                     * value of maxi, but we could check of this
                     * possibility each time a client closes its connection
                     */
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else
                    Writen(sockfd, line, n);

                if (--nready <= 0)
                    continue;   /* no more readable descriptors */
            }
        }
    }

    return 0;
}
