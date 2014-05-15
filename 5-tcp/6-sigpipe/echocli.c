#include <stdio.h>          /* stdin, stdout */
#include <string.h>         /* memset() */
#include <unistd.h>         /* sleep() */
#include <arpa/inet.h>      /* htons() */
#include <netinet/in.h>     /* struct sockaddr_in */
#include <signal.h>         /* SIGPIPE */
#include "common.h"

static void sig_pipe(int signum)
{
    printf("%s: catched the SIGPIPE\n", __func__);
}

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
    while (Fgets(sendline, MAXLINE, fp) != NULL) {
        /* 修改该函数的实现,当杀死服务端子进程后,使客户端能触发SIGPIPE信号.
         * All we have changed is to call writen() two times: the first time
         * the first byte of data is written to the socket, followed by a
         * pause of one second, followed by the remainder of the line. The
         * intent is for the first writen() to elicit the RST and then for
         * the second writen() to generate SIGPIPE.
         */
        Writen(fd, sendline, 1);
        sleep(1);
        /* 当进程接收到SIGPIPE信号,上面的sig_pipe()函数打印出提示信息后,
         * 这个Writen()语句会报错,打印提示信息:"writen error: Broken pipe"
         */
        Writen(fd, sendline + 1, strlen(sendline) - 1);

        if (Readline(fd, recvline, MAXLINE) == 0)
            err_quit("str_cli: server terminated prematurely");

        Fputs(recvline, stdout);
    }
}

/* What happens if the client ignores the error return from readline()
 * and writes more data to the server? This can happen, for example, if the
 * client needs to perform two writes to the server before reading anything
 * back, with the first write eliciting the RST.
 * The rule that applies is: When a process writes to a socket that has
 * received an RST, the SIGPIPE signal is sent to the process. The default
 * action of this signal is to terminate the process, so the process must
 * catch the signal to avoid being involuntarily terminated.
 * If the process either catches the signal and returns from the signal
 * handler, or ignores the signal, the write operation returns EPIPE.
 * It is okey to write to a socket that has received a FIN, but it is an
 * error to write to a socket that has received an RST.
 */
int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in srvaddr;

    if (argc != 2)
        err_quit("usage: echocli <IPaddress>");

    /* 为了方便确认进程接收到了SIGPIPE信号,下面捕获这个信号.书中说是在
     * Linux下,bash会自动打印"Broken pipe"错误信息,但是我试了不行,只好捕获
     * SIGPIPE信号,执行时,上面的函数会打印:"sig_pipe: catched the SIGPIPE"
     */
    Signal(SIGPIPE, sig_pipe);

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &srvaddr.sin_addr);

    Connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

    str_cli(stdin, sockfd);     /* do it all */

    return 0;
}
