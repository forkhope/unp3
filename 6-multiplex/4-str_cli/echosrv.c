#include <stdlib.h>         /* exit() */
#include <errno.h>
#include <string.h>         /* memset() */
#include <unistd.h>
#include <netinet/in.h>     /* struct sockaddr_in */
#include <arpa/inet.h>      /* htonl(), htons() */
#include <signal.h>
#include <sys/wait.h>
#include "common.h"

static void sig_chld(int signum)
{
    pid_t pid;
    int stat;

    /* If there are no terminated children for the process calling wait(),
     * but the process has one or more children that are still executing,
     * then wait() blocks until the first of the existing children
     * terminates.
     *
     * 调用wait()函数可能会导致阻塞,但是在这个例子中,不会出现阻塞的情况.
     * 程序先接收到SIGCHLD信号,此时肯定有子进程结束了,所以再调用wait()函数
     * 时,会立刻获取到已经结束了的子进程号,不会阻塞.
     *
     * 书中提到,调用wait()函数会有问题.多个子进程同时结束会产生多个SIGCHLD
     * 信号,由于Linux的信号不会排队,则只会递送一个SIGCHLD到进程,则这个函数
     * 只会执行一次,wait()函数会返回第一次子进程的进程号,其他结束的子进程将
     * 会变成僵尸进程.还有一种情况时,正在执行该sig_chld()函数时,如果有多个
     * SIGCHLD信号过来,也是只递送一个SIGCHLD信号到进程这里来.
     * 针对这个问题,书中改出的解决方法是循环调用waitpid()函数. We cannot
     * call wait() in a loop, because there is no way to prevent wait()
     * from blocking if there are running children that have not yet
     * terminated.
     *
     * pid_t waitpid(pid_t pid, int *statloc, int options);
     * waitpid() gives us more control over which process to wait for and
     * whether or not to block. First, the pid argument lets us specify the
     * process ID that we want to wait for. A value of -1 says to wait for
     * the first of our children to terminate. The options argument lets us
     * specify additional options. The most common option is WNOHANG. This
     * option tells the kernel not to block if there are no terminated
     * children. In this example, we call waitpid() within a loop, fetching
     * the status of any of our children that have terminated. We must
     * specify the WNOHANG option: This tells waitpid() not to block if
     * there are running children that have not yet terminated.
     */
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        /* Warning: Calling standard I/O functions such as printf() in a
         * signal handler is not recommended. We call printf() here as a
         * diagnostic tool to see when the child terminates.
         */
        printf("child %d terminated, status: %d\n", pid, stat);
    }
}

/* Reads data from the client and echoes it back to the client. */
static void str_echo(int fd)
{
    ssize_t n;
    char buf[MAXLINE];

again:
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
 *
 * 这个程序处理了如下三种场景:
 * 1. We must catch the SIGCHLD signal when forking child processes.
 * 2. We must handle interrupted system calls when we catch signals.
 * 3. A SIGCHLD handler must be coded correctly using waitpid() to
 * prevent any zombies from being left around.
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

    /* 当下面的子进程结束后,如果父进程不做任何处理,子进程将会变成僵尸进程.
     * 书中提到,必须要对僵尸进程做处理.描述如下:
     * Obviously we do not want to leave zombies around. They take up space
     * in the kernel and eventually we can run out of processes. Whenever
     * we fork() children, we must wait() for them to prevent them from
     * becoming zombies. To do this, we establish a signal handler to catch
     * SIGCHLD, and within the handler, we call wait().
     *
     * 处理被中断的系统调用
     * 书中提到,当使用原生的signal()函数时,在Solaris 9上,下面accept()函数
     * 会在处理完SIGCHLD信号后,报错返回,错误码为EINTR.在4.4 BSD上,在处理完
     * SIGCHLD信号后,内核会自动恢复被中断的accept()函数,这个accept()函数不
     * 会报错返回.在Linux下测试,也是这个效果,accept()函数会在中断后自动恢
     * 复,说明Linux原生的signal()函数也指定了SA_RESTART标志位.
     * "man 7 signal"手册提到了,当指定了SA_RESTART标志位时,accept()函数会
     * 自动恢复,如果不指定SA_RESTART标志位,则accept()函数被中断后,会报错
     * 返回,错误码是EINTR.修改下面的Signal()函数的实现,去掉里面的SA_RESTART
     * 标志位,再测试,发现accept()函数确实会报错返回,打印的执行结果如下:
     * child 4649 terminated, status: 0
     * accept error: Interrupted system call
     */
    // signal(SIGCHLD, sig_chld);
    Signal(SIGCHLD, sig_chld);

    for ( ;; ) {
        clilen = sizeof(cliaddr);

        /* The basic rule that applies here is that when a process is
         * blocked in a slow system call and the process catches a signal
         * and the signal handler returns, the system call can return an
         * error of EINTR. Some kernels automatically restart some
         * interrupted system calls. For portability, when we write a
         * program that catches signals (most concurrent servers catch
         * SIGCHLD), we must be prepared for slow system calls to return
         * EINTR. 所以,改成下面的方式来调用accept()函数.这种写法是为了
         * 可移植性和兼容性.实际上Linux原生的signal()函数会让accept()自动
         * 恢复,并不需要这种写法.
         */
        // connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if ((connfd = accept(listenfd,
                        (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue;           /* back to for() */
            else
                err_sys("accept error");
        }

        if ((childpid = Fork()) == 0) {     /* child process */
            Close(listenfd);    /* close listening socket */
            str_echo(connfd);   /* process the request */
            exit(0);
        }
        Close(connfd);      /* parent closes connected socket */
    }

    return 0;
}
