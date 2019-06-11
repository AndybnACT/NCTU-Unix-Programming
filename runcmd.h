#include <sys/user.h>
#include "break.h"

#define STATE_NONE 0x0
#define STATE_LOAD 0x1
#define STATE_RUNNING 0x2
#define STATE_ANY (STATE_LOAD | STATE_RUNNING)
extern int STATE;

struct elf{
    unsigned long long entry;
    unsigned long long vaddr;
    unsigned long long offset;
    long long size;
};

struct disasm {
    char *data;
    int size;
    unsigned long long cur_va;
    int cur;
};

struct prog_ctx {
    char *progname;
    int fd;
    unsigned int pid;
    struct elf load;
    struct user_regs_struct regs;
    // char *codebuf;
    struct disasm asm_file;
    struct disasm asm_raw;
    struct breakpoint *b;
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
int vmmap(int argc, char **argv);

int sdb_run(int argc, char** argv);
int sdb_start(int argc, char** argv);
int sdb_cont(int argc, char** argv);
int sdb_getregs(int argc, char**argv);
int sdb_getreg(int argc, char**argv);
int sdb_setreg(int argc, char**argv);
int sdb_dump(int argc, char **argv);
int sdb_break(int argc, char **argv);
int sdb_listb(int argc, char **argv);
int sdb_delb(int argc, char **argv);
int sdb_si(int argc, char **argv);

int dis_asm(int argc, char** argv);

#define STOP_BREAK_NOT_FOUND 0xFC
#define STOP_EXIT            0xEF