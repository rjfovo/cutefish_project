#define _GNU_SOURCE
#include <signal.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

/*
 * Plymouth normally sends SIGRTMIN+20 / SIGRTMIN+21 to PID 1 when its splash
 * attaches to/detaches from the session. Those signals re-enable systemd's
 * "[ OK ] ..." console status output, which CutefishOS does not want on the
 * visible terminal.
 *
 * This small interposer is loaded into plymouthd only. It swallows exactly
 * those two signals when addressed to PID 1; every other kill() call is
 * passed through to libc unchanged.
 */
typedef int (*kill_fn_t)(pid_t, int);

int kill(pid_t pid, int sig)
{
    static kill_fn_t real_kill = 0;

    if (!real_kill)
        real_kill = (kill_fn_t) dlsym(RTLD_NEXT, "kill");

    if (pid == 1 && (sig == SIGRTMIN + 20 || sig == SIGRTMIN + 21))
        return 0;

    return real_kill(pid, sig);
}
