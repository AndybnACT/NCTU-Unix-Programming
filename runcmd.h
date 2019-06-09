#include <sys/user.h>

#define STATE_NONE 0x0
#define STATE_LOAD 0x1
#define STATE_RUNNING 0x2
#define STATE_ANY (STATE_LOAD | STATE_RUNNING)
extern int STATE;

struct elf{
    unsigned long entry;
    unsigned long vaddr;
    unsigned long offset;
    long size;
};

struct prog_ctx {
    char *progname;
    int fd;
    unsigned int pid;
    struct elf load;
    struct user_regs_struct regs;
};

extern struct prog_ctx prog;

struct command {
    char *name;
    char *shortcut;
    char *help;
    int (*func)(int, char**);
};

#define ARGC_CHK(argc, chk){                                                \
    if (argc != chk) {                                                      \
        printf("Number of argument not match, expect %d but %d is given\n", \
               chk, argc);                                                  \
        return 0;                                                           \
    }                                                                       \
}

int runcmd(char *cmd);
int load_prog(int argc, char **argv);