#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

typedef void Sigfunc(int);

Sigfunc *Signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    /* POSIX allows us to specify a set of signals that will be blocked
     * when our signal handler is called. Any signal that is blocked cannot
     * be delivered to a process. We set the sa_mask member to the empty
     * set, which means that no additional signals will be blocked while
     * our signal handler is running. POSIX guarantees that the signal
     * being caught is always blocked while its handler is executing.
     */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    /* 当传入的信号等于SIGALRM时,不指定SA_RESTART标志 */
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    }
    else {
#ifdef SA_RESTART
        /* SA_RESTART is an optional flag. When the flag is set, a system
         * call interrupted by this signal will be automatically restarted
         * by the kernel. If the signal being caught is not SIGABRT, we
         * specify the SA_RESTART, if defined.
         */
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

static void handle_sigint(int signum)
{
    printf("Enter %s: the signum = %d\n", __func__, signum);
    exit(0);
}

/* POSIX Signal Handling.
 * A signal is a notification to a process that an event has occurred.
 * Signals can be sent
 *      > By one process to another process (or to itself)
 *      > By the kernel to a process
 * The SIGCHLD is one that is sent by the kernel whenever a process
 * terminates, to the pareent of the terminating process.
 *
 * Every signal has a disposition, which is also called the action
 * associated with the signal. We set the disposition of a signal by calling
 * the sigaction() and we have three choices for the disposition:
 * 1. We can provide a function that is called whenever a specific signal
 * occurs. This function is called a signal handler and this action is
 * called catching a signal. The two signals SIGKILL and SIGSTOP cannot be
 * caught. Our function is called with a signal integer argument that is
 * the signal number and the function returns nothing. Its function
 * prototype is therefore "void handler(int signo);"
 * 2. We can ignore a signal by setting its disposition to SIG_IGN. The
 * two signals SIGKILL and SIGSTOP cannot be ignored.
 * 3. We can set the default disposition for a signal by setting its
 * disposition to SIG_DFL. The default is normally to terminate a process
 * on receipt of a signal, with certain signals also generating a core
 * image of the process in its current working directory. There are a few
 * signals whose default disposition is to be ignored: SIGCHLD and SIGURG.
 *
 * POSIX Signal Semantics
 * 1. Once a signal handler is installed, it remains installed. (Older
 * systems removed the signal handler each time it was executed.)
 * 2. While a signal handler is executing, the signal being delivered is
 * blocked. Furthermore, any additional signals that were specified in the
 * sa_mask signal set passed to sigaction() when the handler was installed
 * are also blocked. In Figure 5.6, we set sa_mask to the empty set, meaning
 * no additional signals are blocked other than the signal being caught.
 * 3. If a signal is generated one or more times while it is blocked, it is
 * normally delivered only one time after the signal is unblocked. That is,
 * by default, Unix signals are not queued.
 * 4. It is possible to selectively block and unblock a set of signals
 * using the sigprocmask() function. This lets us protect a critical region
 * of code by preventing certain signals from being caught while that
 * region of code is executing.
 */
int main(void)
{
    /* 测试 Signal() 函数 */
    Signal(SIGINT, handle_sigint);

    for ( ;; ) {
    }

    return 0;
}
