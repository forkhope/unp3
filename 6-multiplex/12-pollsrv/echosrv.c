#include <stdlib.h>         /* exit() */
#include <string.h>         /* memset() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include <arpa/inet.h>      /* htonl(), htons() */
#include <poll.h>           /* poll() */
#include <limits.h>         /* for OPEN_MAX */
#include <unistd.h>
#include <errno.h>
#include "common.h"

/* 书中注释说是包含 limits.h 头文件能够包含OPEN_MAX的声明.但是在Linux上,
 * 包含 limits.h 头文件,并不会声明OPEN_MAX.调试发现,Linux没有定义这个宏.
 * 所以,下面手动定义 OPEN_MAX 宏.
 */
#define OPEN_MAX 1024

/* We now redo our TCP echo server using poll() instead of select() */
int main(void)
{
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    char buf[MAXLINE];
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in srvaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

    Listen(listenfd, LISTENQ);

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (i = 1; i < OPEN_MAX; ++i)
        client[i].fd = -1;     /* -1 indicates available entry */
    maxi = 0;              /* max index into client[] array */

    for ( ;; ) {
        /* Linux 不支持 INFTIM 宏,使用 -1 替代它.实际上, -1 具有更好的
         * 移植性, poll() 函数的第三个参数为负就是表示不超时等待.
         *
         * We call poll() to wait for either a new connection or data on
         * existing connection. When a new connection is accepted, we find
         * the first available entry in the client array by looking for the
         * first one with a negative descriptor. Notice that we start the
         * search with the index of 1, since client[0] is used for the
         * listening socket. When an available entry is found, we save the
         * descriptor and set the POLLRDNORM event.
         */
        nready = Poll(client, maxi + 1, -1);

        if (client[0].revents & POLLRDNORM) {   /* new client connection */
            connfd = Accept(listenfd, NULL, NULL);

            for (i = 1; i < OPEN_MAX; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;     /* save descriptor */
                    break;
                }
            }
            if (i == OPEN_MAX)
                err_quit("too many clients");
            client[i].events = POLLRDNORM;
            if (i > maxi)
                maxi = i;               /* max index in client[] array */

            if (--nready <= 0)
                continue;       /* no more readable descriptors */
        }

        /* Check for data on an existing connection.
         * The two return events that we check for are POLLRDNORM and
         * POLLERR. The second of these we did not set in the events member
         * because it is always returned when the condition is true. The
         * reason we check for POLLERR is because some implementations
         * return this event when an RST is received for a connection,
         * while others just return POLLRDNORM. In either case, we call
         * read() and if an error has occurred, it will return an error.
         * When an existing connection is terminated by the client, we just
         * set the fd member to -1.
         */
        for (i = 1; i <= maxi; ++i) {   /* check all clients for data */
            if ((sockfd = client[i].fd) < 0)
                continue;

            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                if ((n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        /* connection reset by client */
                        Close(sockfd);
                        /* Any entry in the array of pollfd structures
                         * passed to poll() with a negative value for
                         * the fd member is just ignored.
                         */
                        client[i].fd = -1;
                    }
                    else
                        err_sys("read error");
                }
                else if (n == 0) {
                    /* connection closed by client */
                    Close(sockfd);
                    client[i].fd = -1;
                }
                else
                    Writen(sockfd, buf, n);

                if (--nready <= 0)
                    continue;   /* no more readable descriptors */
            }
        }
    }

    return 0;
}
