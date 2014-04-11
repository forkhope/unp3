#ifndef L_COMMON_H_
#define L_COMMON_H_

#define LISTENQ     1024    /* 2nd argument to listen() */

void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);
void err_msg(const char *fmt, ...);

int Socket(int, int, int);
void Bind(int, const struct sockaddr *, socklen_t);
void Listen(int, int);
int Accept(int, struct sockaddr *, socklen_t *);

void Write(int, const void *, size_t);
void Close(int);

ssize_t readn(int fd, void *buf, size_t count)
ssize_t writen(int fd, const void *buf, size_t count)

#endif /* L_COMMON_H_ */

