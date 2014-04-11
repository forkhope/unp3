#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>

static void err_doit(int, int, const char *, va_list);

/* Nonfatal error related to system call
 * Print message and return.
 */
void err_ret(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
}

/* Fatal error related to system call
 * Print message and terminate
 */
void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}

/* Nonfatal error unrelated to system call
 * Print message and return
 */
void err_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
}

/* Fatal error unrelated to system call
 * Print message and terminate
 */
void err_quit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

/* Print message and return to caller
 * Caller specifies "errnoflag"
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[BUFSIZ];

    vsnprintf(buf, BUFSIZ - 1, fmt, ap);
    if (errnoflag)
        snprintf(buf + strlen(buf), BUFSIZ - strlen(buf) - 1,
                ": %s", strerror(error));

    strcat(buf, "\n");
    fflush(stdout);     /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(NULL);       /* flushes all stdin output streams */
}

int Socket(int domain, int type, int protocol)
{
    int n;

    if ((n = socket(domain, type, protocol)) < 0)
        err_sys("socket error");
    return n;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    if (bind(sockfd, addr, len) < 0)
        err_sys("bind error");
}

void Listen(int fd, int backlog)
{
    if (listen(fd, backlog) < 0)
        err_sys("listen error");
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *len)
{
    int n;

    if ((n = accept(sockfd, addr, len)) < 0)
        err_sys("accept error");
    return n;
}

void Write(int fd, const void *buf, size_t count)
{
    if (write(fd, buf, count) != count)
        err_sys("write error");
}

void Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}

/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void *buf, size_t count)
{
    int nleft;
    int nread;
    char *ptr;

    ptr = buf;
    nleft = count;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;      /* and call read() again */
            else
                return -1;
        }
        else if (nread == 0) {
            break;              /* EOF */
        }

        nleft -= nread;
        ptr += nread;
    }
    return count - nleft;           /* return >= 0 */
}

/* Write "n" bytes to a descriptor */
ssize_t writen(int fd, const void *buf, size_t count)
{
    int nleft;
    int nwrite;
    const char *ptr;

    ptr = buf;
    nleft = count;
    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;     /* and call write() again */
            else
                return -1;      /* error */
        }

        nleft -= nwrite;
        ptr += nwrite;
    }

    return count;
}
