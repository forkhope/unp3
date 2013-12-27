#include <stdio.h>
#include <stdlib.h>     /* exit() */
#include <string.h>     /* memset() */
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>  /* inet_pton(), htons() */
#include "common.h"     /* err_sys(), err_quit() */

/* Modify the first argument to socket() in Figure 1.5 to be 9999. Compile
 * and run the program. What happens? Find the errno value corresponding
 * to the error that is printed. How can you find more information on this
 * error?
 */
int main(int argc, char *argv[])
{
    int sockfd, n;
    char recvline[BUFSIZ];
    struct sockaddr_in servaddr;

    if (argc != 2) {
        err_quit("Usage: %s IP-address", argv[0]);
    }
    

    /* 打印出来 AF_INET 的值是 2. 换句话说, socket(AF_INET, SOCK_STREAM, 0)
     * 和 socket(2, SOCK_STREAM, 0)的效果完全是一样的,但由于没有哪个AF_*宏
     * 的值是 9999, 所以下面第一个参数写成 9999 会报错,打印的错误信息为:
     * socket 9999 error: Address family not supported by protocol
     *
     * 查看 man errno 手册,可以发现该错误描述对应的错误码是EAFNOSUPPORT
     * 再查看 man socket 手册,里面对错误码 EAFNOSUPPORT 的描述为:
     * EAFNOSUPPORT: The implementation does not support the specified
     *               address family.
     *
     * 注意:这里的执行结果和书上答案描述的不一样,不知道是否操作系统差异
     * 导致的. 实际上,在我的Linux 3.2上,把socket()的第三个参数设成999,才
     * 是返回书上描述的"Protocol not supported"的错误信息.如后面所示.
     *
     * socket() 函数的第一个参数指定 域(domain), 对应的错误信息确实应该是
     * EAFNOSUPPORT, 表示域不支持. 第三个参数指定 协议(protocol), 所以第三
     * 个参数出错才应该是 EPFNOSUPPORT, 表示协议不支持.
     */
    printf("AF_INET: %d\n", AF_INET);
    if ((sockfd = socket(9999, SOCK_STREAM, 0)) < 0)
        err_ret("socket 9999 error");

    /* 将 socket() 函数的第三个参数设成999,运行将会打印如下的错误信息:
     * socket third-arg 999 error: Protocol not supported
     *
     * 查看 man errno 手册,可以发现该错误描述对应的错误码是EPROTONOSUPPORT
     * 再查看 man socket 手册,里面对错误码 EPROTONOSUPPORT 的描述为:
     * EPROTONOSUPPORT: The protocol type or the specified protocol is
     *                  not supported within this domain.
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 9999)) < 0)
        err_sys("socket third-arg 999 error");

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
    while ((n = read(sockfd, recvline, BUFSIZ)) > 0) {
        recvline[n] = '\0';     /* null terminate */
        if (fputs(recvline, stdout) == EOF)
            err_sys("fputs error");
    }
    if (n < 0)
        err_sys("read error");

    exit(0);
}
