#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include "siglock.h"

void siglock(int act, ...)
{
	static sigset_t set, oset;
	va_list ap;
	int *sig;

	switch (act) {
	case SIGLOCK_INIT:
		sigprocmask(SIG_BLOCK, NULL, &oset);
		sigemptyset(&set);
		va_start(ap, act);
		sig = va_arg(ap, int *);
		va_end(ap);
		while (*sig)
			sigaddset(&set, *sig++);
		break;
	case SIGLOCK_LOCK:
		sigprocmask(SIG_BLOCK, &set, &oset);
		break;
	case SIGLOCK_UNLOCK:
		sigprocmask(SIG_SETMASK, &oset, NULL);
		break;
	default:
		fprintf(stderr, "invalid action of siglock: %d\n", act);
		exit(1);
	}
}