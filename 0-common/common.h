#ifndef L_COMMON_H_
#define L_COMMON_H_

#include <stdio.h>          /* FILE */
#include <poll.h>           /* poll() */

#define MAXLINE     1024
#define LISTENQ     1024    /* 2nd argument to listen() */
#define SERV_PORT   9877

#define min(x,y)   ((x) < (y) ? (x) : (y))
#define max(x,y)   ((x) > (y) ? (x) : (y))

void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);
void err_msg(const char *fmt, ...);

int Socket(int, int, int);
void Connect(int, const struct sockaddr *, socklen_t);
void Bind(int, const struct sockaddr *, socklen_t);
void Listen(int, int);
int Accept(int, struct sockaddr *, socklen_t *);
void Shutdown(int, int);

ssize_t Read(int, void *, size_t);
void Write(int, const void *, size_t);
void Close(int);
pid_t Fork(void);

char *Fgets(char *, int, FILE *);
void Fputs(const char *, FILE *);

void Inet_pton(int, const char *, void *);
const char *Inet_ntop(int, const void *, char *, socklen_t);

int Select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int Poll(struct pollfd *, unsigned long, int);

ssize_t readn(int, void *, size_t);
ssize_t Readn(int, void *, size_t);
ssize_t  Readline(int, void *, size_t);
void Writen(int, void *, size_t);

typedef void Sigfunc(int);
Sigfunc *Signal(int signo, Sigfunc *func);

#endif /* L_COMMON_H_ */

