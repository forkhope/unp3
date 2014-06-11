#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "common.h"

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

void Connect(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    if (connect(sockfd, addr, len) < 0)
        err_sys("connect error");
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

void Shutdown(int fd, int how)
{
    if (shutdown(fd, how) < 0)
        err_sys("shutdown error");
}

ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t n;

    if ((n = read(fd, buf, count)) == -1)
        err_sys("read error");
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

pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    }
    return pid;
}

char *Fgets(char *s, int size, FILE *stream)
{
    char *p;

    if ((p = fgets(s, size, stream)) == NULL && ferror(stream))
        err_sys("fgets error");
    return p;
}

void Fputs(const char *s, FILE *stream)
{
    if (fputs(s, stream) == EOF)
        err_sys("fputs error");
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

static ssize_t my_read(int fd, char *ptr)
{
	static int	read_cnt = 0;
	static char	*read_ptr;
	static char	read_buf[BUFSIZ];

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t Readn(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

    if ((n = readn(fd, ptr, nbytes)) < 0)
        err_sys("readn error");
    return n;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	int		n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			if (n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}

/* Write "n" bytes to a descriptor */
static ssize_t writen(int fd, const void *buf, size_t count)
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

void Writen(int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != nbytes)
        err_sys("writen error");
}

void Inet_pton(int af, const char *src, void *dst)
{
    int n;

    if ((n = inet_pton(af, src, dst)) < 0)
        err_sys("inet_pton error for %s", src); /* errno set */
    else if (n == 0)
        err_quit("inet_pton error for %s", src);    /* errno not set */
}

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    const char *p;

    if ((p = inet_ntop(af, src, dst, size)) == NULL) {
        err_sys("close error");
    }
    return p;
}

Sigfunc *Signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    }
    else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds,
        fd_set *exceptfds, struct timeval *timeout)
{
    int n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
        err_sys("select error");
    return n;   /* can return 0 on timeout */
}

int Poll(struct pollfd *fds, unsigned long nfds, int timeout)
{
    int nready;

    if ((nready = poll(fds, nfds, timeout)) < 0)
        err_sys("poll error");
    return nready;
}
