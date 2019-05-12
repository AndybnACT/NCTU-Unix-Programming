/* Example of using sigaction() to setup a signal handler with 3 arguments
 * including siginfo_t.
 */
// #define __USE_POSIX199309
#include "libmini.h"

#ifdef __USE_POSIX199309
static void hdl (int sig, siginfo_t *siginfo, void *context)
{
	write(1, "signal\n", 8);
}
#else
static void hdl (int sig)
{
	write(1, "signal\n", 8);
}
#endif
 
int main (int argc, char *argv[])
{
	struct sigaction act;
 
	memset (&act, '\0', sizeof(act));
 
	/* Use the sa_sigaction field because the handles has two additional parameters */
	
 
 #ifdef __USE_POSIX199309
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_sigaction = &hdl;
    act.sa_flags = SA_SIGINFO;
 #else
    act.sa_handler = &hdl;
 #endif
 
 
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}
 
	while (1)
		sleep (10);
 
	return 0;
}
