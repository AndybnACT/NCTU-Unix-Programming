#include "libmini.h"
/* This file is copied and modified from glibc-testsuite */
int main(void)
{
	volatile int err = 0;
	volatile int x = 0, r;
	jmp_buf jb;
	jmp_buf sjb;
	volatile sigset_t oldset;
	sigset_t set;

	if (!setjmp(jb)) {
		x = 1;
		longjmp(jb, 1);
        x = 0;
	}
    if (x != 1) {
        write(1, "setjmp/longjmp seems to have been bypassed\n", 46);
    }else{
        write(1, "pass\n", 6);
    }

	x = 0;
	r = setjmp(jb);
	if (!x) {
		x = 1;
		longjmp(jb, 0);
	}

    if (r != 1) {
        write(1, "longjmp(jb, 0) caused set jump to return != 1\n", 47);
    }else{
        write(1, "pass\n", 6);
    }
    
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &set, &set);
	oldset = set;

	/* Improve the chances of catching failure of sigsetjmp to
	 * properly save the signal mask in the sigjmb_buf. */
	memset(&sjb, -1, sizeof sjb);

	if (!setjmp(sjb)) {
		sigemptyset(&set);
		sigaddset(&set, SIGUSR1);
		sigprocmask(SIG_BLOCK, &set, 0);
		longjmp(sjb, 1);
	}
	set = oldset;
	sigprocmask(SIG_SETMASK, &set, &set);
	
    if (sigismember(&set, SIGUSR1)!=0) {
        write(1, "siglongjmp failed to restore mask\n", 35);
    }else{
        write(1, "pass\n", 6);
    }
	return err;
}
