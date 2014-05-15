#include <stdlib.h>         /* exit() */
#include <errno.h>
#include <string.h>         /* memset() */
#include <unistd.h>
#include <netinet/in.h>     /* struct sockaddr_in */
#include <arpa/inet.h>      /* htonl(), htons() */
#include "common.h"

/* Reads data from the client and echoes it back to the client. */
static void str_echo(int fd)
{
    ssize_t n;
    char buf[MAXLINE];

again:
    /* TCP协议面向字节流,read()函数的返回值往往小于参数指定的字节数.对于
     * read()调用,如果TCP接收缓冲区有20个字节,请求读100个字节,则在读取20
     * 个字节后,read()函数就会返回.调用一次read()函数,可能会有如下三种情况:
     * (1)TCP接收缓冲区中没有数据,则read()函数阻塞,直到缓冲区中有数据为止.
     * (2)缓冲区中的数据长度小于read()函数指定的字节数,则read()取走缓冲区中
     * 的所有数据,然后返回.此时返回值会小于第三个参数指定的值.
     * (3)缓冲区中的数据长度大于read()指定的字节数,则read()填满buffer后返回
     *
     * 在执行这个程序时,可以看到客户端每输入一行,立刻就会回显一行,也就是客
     * 户端输入一行数据后,下面的read()函数读取这一行数据就立刻返回,然后调用
     * write()回显给客户端.这里的read()函数并不是在读满MAXLINE个字节才返回,
     * 也不是读取到客户端输入的换行符才返回.read()并不对换行符做特别处理.
     */
    while ((n = read(fd, buf, MAXLINE)) > 0)
        Writen(fd, buf, n);

    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        err_sys("str_echo: read error");
}

/* Out simple example is an echo server that performs the following steps:
 * 1. The client reads a line of text from its standard input and writes
 * the line to the server.
 * 2. The server reads the line from its network input and echoes the line
 * back to the client.
 * 3. The client reads the echoed line and prints it on its standard output
 */
int main(void)
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, srvaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

    Listen(listenfd, LISTENQ);

    for ( ;; ) {
        clilen = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

        if ((childpid = Fork()) == 0) {
            Close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        Close(connfd);
    }

    return 0;
}
