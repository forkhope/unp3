#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "common.h"

/* accept() is called by a TCP server to return the next completed
 * connection from the front of the completed connection queue. If the
 * completed connection queue is empty, the process is put to sleep
 * (assuming the default of a blocking socket).
 * #include <sys/socket.h>
 * int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
 *          Returns: non-negative descriptor if OK, -1 on error
 * The cliaddr and addrlen arguments are used to return the protocol address
 * of the connected peer process (the client). addrlen is a value-result
 * argument: Before the call, we set the integer value referenced by
 * *addrlen to the size of the socket address structure pointed to by
 * cliaddr, on return, this integer value contains the actual number of
 * bytes stored by the kernel in the socket address structure.
 * If accept() is successful, its return value is a brand-new descriptor
 * automatically created by the kernel. This new descriptor refers to the
 * TCP connection with the client. When discussing accept(), we call the
 * first argument to accept() the listening socket (the descriptor created
 * by socket() and then used as the first argument to both bind() and
 * listen()), and we call the return value from accept() the connected
 * socket. A given server normally creates only one listening socket, which
 * then exists for the lifetime of the server. The kernel creates one
 * connected socket for each client connection that is accepted (i.e., for
 * which the TCP three-way handshake completes). When the server is finished
 * serving a given client, the connected socket is closed.
 *
 * This function returns up to three values: an integer return code that is
 * either a new socket descriptor or an error indication, the protocol
 * address of the client process (through the cliaddr pointer), and the size
 * of this address (through the addrlen pointer). If we are not interested
 * in having the protocol address of the client returned, we set both
 * cliaddr and addrlen to null pointers.
 */
int main(void)
{
    int listenfd, connfd;
    socklen_t len;
    struct sockaddr_in srvaddr, cliaddr;
    char buff[MAXLINE];
    time_t ticks;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(13);   /* daytime server */

    /* 这里要绑定端口13,而这个端口属于保留端口号,需要超级用户权限才能绑定.
     * Our server must run with superuser privileges to bind() the reserved
     * port of 13. If we do not have superuser privileges, the call to
     * bind() will fail: "bind error: Permission denied"
     *
     * 书中Exercises 4.5提到这样一个问题: Remove the call to bind(), but
     * allow the call to listen(). What happens? 书中答案是: Without a call
     * to bind(), the call to listen() assigns en ephemeral port to the
     * listening socket.此时,IP地址应该也是INADDR_ANY.
     */
    Bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

    /* 如果没有这一句,执行的时候会报错: "accept error: Invalid argument".
     * 该错误信息对应的错误码是EINVAL.查看man accept手册,里面有如下描述:
     * EINVAL   Socket is not listening for connections, or addrlen is
     *          invalid (e.g., is negative).
     * 书中Exercises 4.4提到了这种情况.
     */
    Listen(listenfd, LISTENQ);

    for (;;) {
        /* 接收客户端的连接请求,并获取地址信息,打印出IP地址和端口号.
         * 在客户端执行"daytimecli 127.0.0.1"和"daytimecli 192.168.1.127"
         * 时,这个服务端的程序的打印结果如下:
         * connection from 127.0.0.1, port 47989
         * connection from 192.168.1.127, port 45741
         * 对此,书中有所描述: Notice what happens with the client's IP
         * address. Since our daytime client does not call bind(), the
         * kernel chooses the source IP address based on the outgoing
         * interface that is used. In the first case, the kernel sets the
         * source IP address to the loopback address; in the second case,
         * it sets the address to the IP address of the Ehternet interface.
         */
        len = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        printf("connection from %s, port %d\n",
                Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
                ntohs(cliaddr.sin_port));

        /* 将当前的时间信息写入到客户端 */
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        Write(connfd, buff, strlen(buff));

        Close(connfd);
    }

    return 0;
}
