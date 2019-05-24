#include "libmini.h"

long priv_errno;

#define	WRAPPER_RETval(type)	priv_errno = 0; if(ret < 0) { priv_errno = -ret; return -1; } return ((type) ret);
#define	WRAPPER_RETptr(type)	priv_errno = 0; if(ret < 0) { priv_errno = -ret; return NULL; } return ((type) ret);

ssize_t memset(void* dst, char set, ssize_t cnt){
    char *dest = (char*) dst;
    for (size_t i = 0; i < cnt; i++) {
        dest[i] = set;
    }
    return cnt;
}
#define sigmask(sig) \
    (((sigset_t) 0x1) << (sig-1)) 

ssize_t	read(int fd, char *buf, size_t count) {
	long ret = sys_read(fd, buf, count);
	WRAPPER_RETval(ssize_t);
}

ssize_t	write(int fd, const void *buf, size_t count) {
	long ret = sys_write(fd, buf, count);
	WRAPPER_RETval(ssize_t);
}

/* open is implemented in assembly, because of variable length arguments */

int	close(unsigned int fd) {
	long ret = sys_close(fd);
	WRAPPER_RETval(int);
}

void *	mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) {
	long ret = sys_mmap(addr, len, prot, flags, fd, off);
	WRAPPER_RETptr(void *);
}

int	mprotect(void *addr, size_t len, int prot) {
	long ret = sys_mprotect(addr, len, prot);
	WRAPPER_RETval(int);
}

int	munmap(void *addr, size_t len) {
	long ret = sys_munmap(addr, len);
	WRAPPER_RETval(int);
}

sighandler_t signal(int signum, sighandler_t handler){
    long ret;
    long sys_ret;
    struct ksigaction oact = {0,};
    struct ksigaction nact = {0,};
    // basic check
    if (signum < 0 || signum > SIGIO || signum == SIGKILL || signum == SIGSTOP) {
        ret = -EINVAL;
        WRAPPER_RETptr(sighandler_t);
    }
    // prepare ksigaction
    nact.sa_handler = handler;
    nact.sa_flags = SA_RESTORER;
    nact.sa_restorer = (__sigrestore_t) __restore;
    
    sys_ret = sys_rt_sigaction(signum, &nact, &oact, sizeof(sigset_t));
    
    // parse return
    if (sys_ret < 0) {
        ret = sys_ret;
        WRAPPER_RETptr(sighandler_t);
    }else{
        ret = (long) oact.sa_handler;
        WRAPPER_RETptr(sighandler_t);
    }
    
}
// Please notice that the sigaction data structure used in the C library may
// be different from that used in the kernel. If the user space and the kernel 
// space data structure are inconsistent, you will have to perform the 
// convertion in your wrapper functions.
int sigaction(int signum, struct sigaction *act, struct sigaction *oldact){
    long ret;
    struct ksigaction koact = {0,};
    struct ksigaction knact = {0,};
    if (act) {
        knact.sa_handler = act->sa_handler;
        knact.sa_mask = act->sa_mask;
        knact.sa_flags = act->sa_flags | SA_RESTORER;
        knact.sa_restorer = (__sigrestore_t) __restore;
    }

    ret = sys_rt_sigaction(signum, &knact, &koact, sizeof(sigset_t));
    
    if (ret >= 0 && oldact) {
        oldact->sa_handler = koact.sa_handler;
        oldact->sa_mask = koact.sa_mask;
        oldact->sa_flags = koact.sa_flags;
        oldact->sa_restorer = (void (*)(void)) koact.sa_restorer;
    }
    WRAPPER_RETval(int);
}

int sigprocmask(int how, sigset_t *set, sigset_t *oldset){
    long ret = sys_rt_sigprocmask(how, set, oldset, sizeof(sigset_t));
    WRAPPER_RETval(int);
}
int sigemptyset(sigset_t *set){
    long ret = 0;
    if (!set) {
        ret = -EINVAL;
    }else{
        memset(set, 0, sizeof(sigset_t));
    }
    WRAPPER_RETval(int);
}
int sigfillset(sigset_t *set){
    long ret = 0;
    if (!set) {
        ret = -EINVAL;
    }else{
        memset(set, 0xFF, sizeof(sigset_t));
    }
    WRAPPER_RETval(int);
}
int sigaddset(sigset_t *set, int signum){
    long ret = 0;
    if (signum < 0 || signum > SIGIO || !set) {
        ret = -EINVAL;
        WRAPPER_RETval(int);
    }
    *set |= sigmask(signum);
    WRAPPER_RETval(int);
}
int sigdelset(sigset_t *set, int signum){
    sigset_t oldset = *set;
    long ret = 0;
    if (signum < 0 || signum > SIGIO || !set) {
        ret = -EINVAL;
        WRAPPER_RETval(int);
    }
    *set &= ~sigmask(signum);
    WRAPPER_RETval(int);
}
int sigismember(const sigset_t *set, int signum){
    unsigned long ret;
    sigset_t test_sig = sigmask(signum);
    if (!set) {
        ret = -EINVAL;
        WRAPPER_RETval(int);
    }
    if (*set & test_sig) {
        return 1;
    }else {
        return 0;
    }
}
int	pipe(int *filedes) {
	long ret = sys_pipe(filedes);
	WRAPPER_RETval(int);
}

int	dup(int filedes) {
	long ret = sys_dup(filedes);
	WRAPPER_RETval(int);
}

int	dup2(int oldfd, int newfd) {
	long ret = sys_dup2(oldfd, newfd);
	WRAPPER_RETval(int);
}

int	pause() {
	long ret = sys_pause();
	WRAPPER_RETval(int);
}

int	nanosleep(struct timespec *rqtp, struct timespec *rmtp) {
	long ret = nanosleep(rqtp, rmtp); //??
	WRAPPER_RETval(int);
}

unsigned int alarm(unsigned int seconds){
    long ret = sys_alarm(seconds);
    WRAPPER_RETval(unsigned int);
}

pid_t	fork(void) {
	long ret = sys_fork();
	WRAPPER_RETval(pid_t);
}

void	exit(int error_code) {
	sys_exit(error_code);
	/* never returns? */
}

char *	getcwd(char *buf, size_t size) {
	long ret = sys_getcwd(buf, size);
	WRAPPER_RETptr(char *);
}

int	chdir(const char *pathname) {
	long ret = sys_chdir(pathname);
	WRAPPER_RETval(int);
}

int	rename(const char *oldname, const char *newname) {
	long ret = sys_rename(oldname, newname);
	WRAPPER_RETval(int);
}

int	mkdir(const char *pathname, int mode) {
	long ret = sys_mkdir(pathname, mode);
	WRAPPER_RETval(int);
}

int	rmdir(const char *pathname) {
	long ret = sys_rmdir(pathname);
	WRAPPER_RETval(int);
}

int	creat(const char *pathname, int mode) {
	long ret = sys_creat(pathname, mode);
	WRAPPER_RETval(int);
}

int	link(const char *oldname, const char *newname) {
	long ret = sys_link(oldname, newname);
	WRAPPER_RETval(int);
}

int	unlink(const char *pathname) {
	long ret = sys_unlink(pathname);
	WRAPPER_RETval(int);
}

ssize_t	readlink(const char *path, char *buf, size_t bufsz) {
	long ret = sys_readlink(path, buf, bufsz);
	WRAPPER_RETval(ssize_t);
}

int	chmod(const char *filename, mode_t mode) {
	long ret = sys_chmod(filename, mode);
	WRAPPER_RETval(int);
}

int	chown(const char *filename, uid_t user, gid_t group) {
	long ret = sys_chown(filename, user, group);
	WRAPPER_RETval(int);
}

int	umask(int mask) {
	long ret = sys_umask(mask);
	WRAPPER_RETval(int);
}

int	gettimeofday(struct timeval *tv, struct timezone *tz) {
	long ret = sys_gettimeofday(tv, tz);
	WRAPPER_RETval(int);
}

uid_t	getuid() {
	long ret = sys_getuid();
	WRAPPER_RETval(uid_t);
}

gid_t	getgid() {
	long ret = sys_getgid();
	WRAPPER_RETval(uid_t);
}

int	setuid(uid_t uid) {
	long ret = sys_setuid(uid);
	WRAPPER_RETval(int);
}

int	setgid(gid_t gid) {
	long ret = sys_setgid(gid);
	WRAPPER_RETval(int);
}

uid_t	geteuid() {
	long ret = sys_geteuid();
	WRAPPER_RETval(uid_t);
}

gid_t	getegid() {
	long ret = sys_getegid();
	WRAPPER_RETval(uid_t);
}

int sigpending(sigset_t *set){
    long ret = sys_rt_sigpending(set, sizeof(sigset_t));
    WRAPPER_RETval(int);
}

void setsigjmp(jmp_buf env) {
    sigset_t oldset;
    long ret = sys_rt_sigprocmask(0x0, NULL, &oldset, sizeof(sigset_t));
    if (ret < 0) {
        priv_errno = -ret;
        perror("signal mask");
    }
    env->mask = oldset;
    return;
}
void longsigjmp(jmp_buf env) {
    long ret = sys_rt_sigprocmask(SIG_SETMASK, &env->mask, NULL, sizeof(sigset_t));
    if (ret < 0) {
        priv_errno = -ret;
        perror("signal mask");
    }
    return;
}

void bzero(void *s, size_t size) {
	char *ptr = (char *) s;
	while(size-- > 0) *ptr++ = '\0';
}

size_t strlen(const char *s) {
	size_t count = 0;
	while(*s++) count++;
	return count;
}

#define	PERRMSG_MIN	0
#define	PERRMSG_MAX	34

static const char *errmsg[] = {
	"Success",
	"Operation not permitted",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Argument list too long",
	"Exec format error",
	"Bad file number",
	"No child processes",
	"Try again",
	"Out of memory",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device or resource busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Math argument out of domain of func",
	"Math result not representable"
};

void perror(const char *prefix) {
	const char *unknown = "Unknown";
	long backup = priv_errno;
	if(prefix) {
		write(2, prefix, strlen(prefix));
		write(2, ": ", 2);
	}
	if(priv_errno < PERRMSG_MIN || priv_errno > PERRMSG_MAX) write(2, unknown, strlen(unknown));
	else write(2, errmsg[backup], strlen(errmsg[backup]));
	write(2, "\n", 1);
	return;
}

#if 0	/* we have an equivalent implementation in assembly */
unsigned int sleep(unsigned int seconds) {
	long ret;
	struct timespec req, rem;
	req.tv_sec = seconds;
	req.tv_nsec = 0;
	ret = sys_nanosleep(&req, &rem);
	if(ret >= 0) return ret;
	if(ret == -EINTR) {
		return rem.tv_sec;
	}
	return 0;
}
#endif
