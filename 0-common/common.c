#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

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
static void err_doit(int error, int errnoflag, const char *fmt, va_list ap)
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
