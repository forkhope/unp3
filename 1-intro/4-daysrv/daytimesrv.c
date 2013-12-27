#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>  /* htons(), htonl() */
#include <time.h>       /* time(), ctime() */
#include "common.h"

/* We can write a simple version of a TCP daytime server, which will work
 * with the daytime client.
 *
 * TCP服务器端一般要执行的步骤是: socket()->填充结构体(协议类型,本机ip地址,
 * 本机端口)->bind()->listen()->accept()->使用连接描述符进行读写.
 * TCP客户端一般要执行的步骤是: socket()->填充结构体(协议类型,服务器ip地址,
 * 服务器已知端口)->connect()
 */
int main(void)
{
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buf[BUFSIZ];
    time_t ticks;

    /* 1. Create a TCP socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 2. Bind server's well-known port to socket
     * The server's well-known port (13 for the daytime service) is bound
     * to the socket by filling in an Internet socket address structure and
     * calling bind(). We specify the IP address as INADDR_ANY, which allows
     * the server to accept a client connection on any interface, in case
     * the server host has multiple interfaces.
     *
     * 以普通用户运行该程序时,会报错: bind error: Permission denied
     * 此时,错误码是EACCES. 查看man bind手册,对该错误码解释为:
     * EACCES The address is protected, and the user is not the superuser.
     *
     * 在Linux中,只有 root 用户可以绑定 1024 以下的端口.
     * 查看 man 7 ip 手册,里面有如下说明:
     * The port numbers below 1024 are called privileged ports (or
     * sometimes: reserved ports). Only privileged processes (i.e., those
     * having the CAP_NET_BIND_SERVICE capability) may bind to these sockets
     * 所以运行该程序时,要用root用户或者sudo命令来运行.
     * 另外, 13 这个端口可能被占用导致绑定不成功. Linux下,查看端口是否被占
     * 用的方法是: netstat -apn | grep <端口号>. 例如: netstat -apn|grep 13
     */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(13);
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    /* 3. Convert socket to listening socket
     * By calling listen(), the socket is converted into a listening socket,
     * on which incomming connections from clients will be accepted by the
     * kernel. These three steps, socket(), bind(), and listen(), are the
     * normal steps for any TCP server to prepare what we call the listening
     * descriptor (listenfd in this example).
     */
    Listen(listenfd, LISTENQ);

    for (; ;) {
        /* 4. Accept client connection, send reply
         * Normally, the server process is put to sleep in the call to
         * accept(), waiting for a client connection to arrive and be
         * accepted. A TCP connection uses what is called a three-way
         * handshake to establish a connection. When this handshake
         * completes, accept() returns, and the return value from the
         * function is a new descriptor (connfd) that is called the
         * connected descriptor. This new descriptor is used for
         * communication with the new client. A new descriptor is returned
         * by accept() for each client that connects to our server.
         */
        connfd = Accept(listenfd, NULL, NULL);

        /* The current time and date are returned by the library function
         * time(), which returns the number of seconds since the Unix
         * Eproch: 00:00:00 January 1, 1970, Coordinated Universal Time(UTC)
         * The next library function, ctime(), converts this integer value
         * into a human-readable string sush as: Mon May 26 20:58:40 2003
         */
        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        Write(connfd, buf, strlen(buf));

        /* 5. Terminate connection
         * The server closes its connection with the client by calling
         * close(). This initiates the normal TCP connection termination
         * sequence: a FIN is sent in each direction and each FIN is
         * acknowledged by the other end.
         */
        Close(connfd);
    }

    return 0;
}
