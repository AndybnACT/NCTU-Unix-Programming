#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "debug.h"
#include "util.h"
#include "runcmd.h"
#include "break.h"

int tracee_startup(unsigned long);
int tracee_cont(void);
int tracee_getallregs(void);
int tracee_setallregs(void);
int tracee_setmem(unsigned long long addr, 
                unsigned long long *data,
                unsigned long long **dataptr,
                int nrdata );
// int tracee_getmem(unsigned long long addr, 
//                 unsigned long long *data,
//                 unsigned long long **dataptr,
//                 int nrdata );
int tracee_setbreak(struct breakpoint *b);
int tracee_revisebreak(struct breakpoint *b);
int tracee_si_safe(void);

int sdb_run(int argc, char** argv){
    ARGC_CHK(argc, 1);
    if (STATE & STATE_RUNNING) {
        printf("** program %s is already running, continuing\n", prog.progname);
        return sdb_cont(1, argv);
    }
    if (!(STATE & STATE_LOAD)) {
        printf("** program not loaded!\n");
        return 0;
    }
    
    return tracee_startup(0);
}

int sdb_cont(int argc, char** argv){
    ARGC_CHK(argc, 1);
    // check state
    if (!(STATE & STATE_RUNNING)) {
        printf("** cont command can only be used at running state\n");
        return 0;
    }
    // continue
    return tracee_cont();
}

int sdb_start(int argc, char** argv){
    ARGC_CHK(argc, 1);
    if (STATE != STATE_LOAD) {
        printf("** start command can only be called at loaded state\n");
        return 0;
    }
    return tracee_startup(1);
}

int sdb_getregs(int argc, char**argv){
    ARGC_CHK(argc, 1);
    if (!(STATE & STATE_RUNNING)) {
        printf("** there is no debugged process running\n");
        return 0;
    }
    if(tracee_getallregs())
        return -1;
    
    dump_all_regs(&prog.regs);
    
    return 0;
};

int sdb_getreg(int argc, char **argv){
    ARGC_CHK(argc, 2);
    if (!(STATE & STATE_RUNNING)) {
        printf("** there is no running debugged process \n");
        return 0;
    }
    if(tracee_getallregs())
        return -1;
    
    if (dump_reg(&prog.regs, argv[1])) {
        printf("** try getregs to see available registers provided by sdb\n");
    }

    return 0;
}

int sdb_setreg(int argc, char **argv){
    unsigned long long val;
    ARGC_CHK(argc, 3);
    if (!(STATE & STATE_RUNNING)) {
        printf("** there is no running debugged process \n");
        return 0;
    }
    // get regs
    if(tracee_getallregs())
        return -1;
    // str2num
    val = str2num(argv[2]);
    if (val == -1 && errno) {
        printf("** cannot recognize input number\n");
        return 0;
    }
    // setreg
    if (set_reg(&prog.regs, argv[1], val)) {
        printf("** try getregs to see available registers provided by sdb\n");
        return 0;
    }
    // commit
    if (tracee_setallregs())
        return -1;

    return 0;
}

int sdb_dump(int argc, char **argv){
    unsigned long long *dump;
    unsigned long long addr = prog.sdb_dumpaddr;
    long long int len = 80;
    unsigned long long buflen, nrlong;
    unsigned long long *dumpped;
    // check state
    if (!(STATE & STATE_RUNNING)) {
        printf("** there is no running debugged process \n");
        return 0;
    }
    // parse addr
    if (argc == 1) {
        if (addr == -1) {
            printf("** no addr is given\n");
            return 0;
        }
    }else if (argc == 2 || argc == 3 ) {
        addr = str2num(argv[1]);
        if (addr == -1 && errno) {
            printf("** Cannot recognize address: %s\n", argv[1]);
            return 0;
        }
        if (argc == 3){
            len = str2num(argv[2]);
            if (len <= 0) {
                printf("** invalid length: %s\n", argv[2]);
                return 0;
            }
        }
    }else{
        ARGC_CHK(argc, 2);
    }
    // get data
    buflen = (len+7)&(~0x7);
    nrlong = buflen/8;
    dump = (unsigned long long *) malloc(buflen);
    if (!dump) {
        perror("cannot allocate buffer for dumpping ");
        return 0;
    }
    dprintf(0, "buf size %d, %d\n", buflen, nrlong);
    if (tracee_getmem(addr, dump, &dumpped, nrlong)) {
        printf("** cannot fully dump the specified memory region\n");
    }
    // display
    long long size = (long long)((char*)dumpped - (char*)dump);
    dprintf(0, "size  %d\n", size );
    size = size > len ? len : size;
    dump_hex((char*) dump, addr, size);
    // success, update addr
    addr += size;
    prog.sdb_dumpaddr = addr;
    free(dump);
    return 0;
}

int sdb_break(int argc, char **argv){
    ARGC_CHK(argc, 2);
    struct breakpoint *new;
    if (!(STATE & STATE_ANY)) {
        printf("** program not loaded\n");
        return 0;
    }
    
    if (!(new = break_init())) {
        printf("cannot create metadata for the new breakpoint\n");
        return -1;
    }
    // get address
    new->addr = str2num(argv[1]);
    if (new->addr == -1 && errno) {
        printf("** cannot recognize input number\n");
        goto failed_free;
    }
    
    
    // commit to breakpoint list, check if the bp exist at the same time
    if (break_insert(&prog.b, new))
        goto failed_free;


    //set breakpoint 
    if (STATE & STATE_RUNNING) {
        if (tracee_setbreak(new)) { // activate
            printf("** error setting breakpoint\n");
            goto failed_free;
        }
    }
    new->enable = 1;
    new->activate = tracee_setbreak;
    new->deactivate = tracee_revisebreak;
    new->dis = capstone_dis_break;
    
    if (new->addr > prog.memoff + prog.load.vaddr + prog.load.size ||
        new->addr < prog.memoff + prog.load.vaddr
    ) {
        printf("** warning, breaking at an address out of loaded .text section of the source itself\n");
        if (prog.is_dyn) 
            printf("** the breakpoint may not preserve accross each run due to ASLR\n");
    }
    
    return 0;

failed_free:
    free(new);
    return 0;    
}

int sdb_listb(int argc, char **argv){
    ARGC_CHK(argc, 1);
    dbg_dump_breaks(CONFIG_DEBUG_LEVEL+1, prog.b);
    return 0;
}

int sdb_delb(int argc, char **argv){
    int id;
    ARGC_CHK(argc, 2);
    
    id = str2num(argv[1]);
    if (id < 0) {
        printf("** Invalid id: %s\n", argv[1]);
        return 0;
    }
    
    if (break_remove_by_id(&prog.b, id))
        printf("cannot find breakpoint id with %d\n", id);
    
    return 0;
}

int sdb_si(int argc, char **argv){
    ARGC_CHK(argc, 1);
    
    if (!(STATE & STATE_RUNNING)) {
        printf("** program must be started before it can be single-stepped\n");
        return 0;
    }
    
    return tracee_si_safe();
}
//========== ptrace related

int tracee_getallregs(void){
    int ret;
    pid_t child = prog.pid;
    ret = ptrace(PTRACE_GETREGS, child, 0, &prog.regs);
    if (ret == -1) {
        perror("ptrace getregs ");
        return -1;
    }
    return 0;
}

int tracee_setallregs(void){
    int ret;
    pid_t child = prog.pid;
    ret = ptrace(PTRACE_SETREGS, child, 0, &prog.regs);
    if (ret == -1) {
        perror("ptrace settregs ");
        return -1;
    }
    return 0;
}

int tracee_setmem(unsigned long long addr, 
                  unsigned long long *data,
                  unsigned long long **dataptr, // used to indicate the final pos
                  int nrdata){
    pid_t child = prog.pid;
    int ret;
    *dataptr = data;
    for (size_t i = 0; i < nrdata; i++) {
        ret = ptrace(PTRACE_POKEDATA, child, (void*) addr, *data);
        if (ret == -1)
            goto failed;
        addr += 8;
        data += 1;
        *dataptr += 1;
    }
    return 0;
failed:
    perror("ptrace peekdata ");
    return -1;
}

int tracee_getmem(unsigned long long addr, 
                  unsigned long long *data,
                  unsigned long long **dataptr,
                  int nrdata ){
    pid_t child = prog.pid;
    *dataptr = data;
    for (size_t i = 0; i < nrdata; i++) {
        errno = 0;
        *data = ptrace(PTRACE_PEEKDATA, child, (void*) addr, NULL);
        if (*data == -1 && errno)
            goto failed;
        addr += 8;
        data += 1;
        *dataptr += 1;
    }
    return 0;
failed:
    perror("ptrace peekdata ");
    return -1;
}


int tracee_setbreak(struct breakpoint *b){
    unsigned long long *ptr;
    unsigned long long trap;
    // preserve tracee's original code
    if (tracee_getmem(b->addr, &b->data, &ptr, 1))
        goto failed;
    trap = b->data;
    trap &= 0xffffffffffffff00;
    trap |= 0xcc;
    // modify code area
    if (tracee_setmem(b->addr, &trap, &ptr, 1))
        goto failed;
    
    b->activated = 1;
    return 0;
failed:
    dbg_show_break(0, b);
    return -1;
}

int tracee_revisebreak(struct breakpoint *b){
    unsigned long long *ptr;
    unsigned long long trap;
    // get tracee's (broken) code
    if (tracee_getmem(b->addr, &trap, &ptr, 1))
        goto failed;
        
    trap &= 0xffffffffffffff00;
    trap |= b->data & 0xFF;
    
    // revise to original
    if (tracee_setmem(b->addr, &trap, &ptr, 1))
        goto failed;
    
    b->activated = 0;
    return 0;
failed:
    dbg_show_break(0, b);
    return -1;
}

int tracee_sig(void){
    pid_t child = prog.pid;
    siginfo_t data;
    if (ptrace(PTRACE_GETSIGINFO, child, 0, &data)) {
#ifdef DEBUG
        perror("ptrace getsig");
#endif
        return -1;
    }
#ifdef DEBUG
    psiginfo(&data, "child ");
#endif
    return data.si_signo;
}

int tracee_stop_callback(int status){
    int sig;
    sig = tracee_sig();
    prog.sig = 0;
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        int exitcode = WEXITSTATUS(status);
        if (WIFEXITED(status))
            printf("** child process %d terminated %s (code %d)\n",
                    prog.pid, 
                    (exitcode == 0)? "normally":"abnormally",
                    exitcode);
        else
            printf("** child process %d terminated by signal %d\n",
                    prog.pid, WTERMSIG(status));
    
        STATE &= ~STATE_RUNNING;
        break_unmask_all(prog.b);
        break_clear_activate_all(prog.b);
        break_set_offset_all(prog.b, -prog.memoff);
        prog.asm_file.dumpped = 0;
        prog.asm_raw.dumpped = 0;
        prog.pid = -1;
        return STOP_EXIT;
    }else if (WIFSTOPPED(status) && sig == SIGTRAP) {
        dprintf(0, "stop status = 0x%x\n", WSTOPSIG(status));
        // get rip
        tracee_getallregs();
        unsigned long long rip = prog.regs.rip - 1;
        
        // handle breakpoint
        dprintf(0, "child process stopped, checking breakpoints\n");
        if (break_hit(prog.b, rip)) {
            dprintf(0, "child is stopped but not stopped by any breakpoint\n");
            dprintf(0, "tracee_stop_callback is not fully implemented!\n");
            dbg_dump_breaks(0, prog.b);
            return STOP_BREAK_NOT_FOUND;
        }
        // reset rip
        prog.regs.rip = rip; // ugly
        tracee_setallregs();
        return STOP_BREAK;
    }else if (sig > 0) {
        if (WIFSTOPPED(status))
            dprintf(0, "stop status = 0x%x\n", WSTOPSIG(status));
        psignal(sig, "** child receives ");
        switch (sig) {
            case SIGSEGV:
            case SIGSTOP:
            case SIGTSTP:
                break;
            default:
                prog.sig = sig;
        }
        return STOP_SIGNALED;
    }
    dprintf(0, "tracee_stop_callback not implemented!\n");
    return 0;
}

int tracee_si(void){
    pid_t child = prog.pid;
    int status;
    ptrace(PTRACE_SINGLESTEP, child, 0, prog.sig);
    if (waitpid(child, &status, 0) < 0) {
        perror("waitpid");
    }
    return tracee_stop_callback(status);
}

int tracee_si_safe(void){
    int reason;
    reason = tracee_si();
    
    switch (reason) {
        case STOP_EXIT:
            return STOP_EXIT;
        case STOP_BREAK_NOT_FOUND:
            break_unmask_all(prog.b);
            break_activate_all(prog.b);
            return 0;
        case STOP_BREAK:
            break_activate_all(prog.b);
            return STOP_BREAK;
        case STOP_SIGNALED:
            return STOP_SIGNALED;
        default:
            dprintf(0, "bug, tracee stop with un known reason\n");
    }
    return 0;
}

int tracee_cont(void){
    pid_t child = prog.pid;
    int status;
    
    if (tracee_si_safe())
        return 0;
    
    ptrace(PTRACE_CONT, child, 0, prog.sig);
    if (waitpid(child, &status, 0) < 0) {
        perror("waitpid ");
    }
    return tracee_stop_callback(status);
}

int tracee_set_memoff(pid_t p){
    unsigned long long st, ed, off, a2;
    char procname[255];
    char dev[255];
    char filename[255];
    char *prog_name;
    char perm[10];
    FILE *f;
    int i;
    
    if (!prog.is_dyn) {
        prog.memoff = 0;
        return 0;
    }
    
    snprintf(procname, 255, "/proc/%d/maps", p);
    
    dprintf(1, "opening proc file\n");
    f = fopen(procname, "r");
    if (!f) {
        perror("error opening file ");
        return -1;
    }
    
    for (i = strlen(prog.progname) - 1; i >= 0; i--) {
        if (prog.progname[i] == '/')
            break;
    }
    prog_name = prog.progname + i;
    
    prog.memoff = 0;
    while (read_mapping(f, &st, &ed, perm, &off, dev, &a2, filename)){
        if (perm[2] == 'x' && strstr(filename, prog_name)) {
            prog.memoff = st;
            break;
        }else{
            dprintf(0, "finding relocated address of the program\n");
        }
    }
    if (!prog.memoff) {
        printf("** Error, cannot find relocated addr of the program\n");
    }
    
    if (off) {
        printf("** Warning, loaded offset %llx != 0 \n", off);
        prog.memoff -= off;
    }
    
    fclose(f);
    dprintf(0, "program is actually loaded at %llx due to ASLR\n", prog.memoff);
    return 0;
}



int tracee_startup(unsigned long stop){
    pid_t child;
    if ((child = fork()) < 0) {
        perror("error forking tracee ");
        return -1;
    }
    
    if (child == 0) {
        // child
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            perror("tracee: ptrace error: ");
            exit(EXIT_FAILURE);
        }
        char *argv[] = {NULL, NULL};
        argv[0] = prog.progname; 
        if (execv(prog.progname, argv) == -1){
            perror("tracee: execv error: ");
            exit(EXIT_FAILURE);
        };
    }else{
        // parent
        int status;
        STATE |= STATE_RUNNING;
        printf("** pid %d\n", child);
        prog.pid = child;
        if (waitpid(child, &status, 0) < 0) {
            perror("waitpid: ");
        }
        status = tracee_stop_callback(status);
        if (status == STOP_EXIT)
            return 0;
        tracee_set_memoff(child);
        break_set_offset_all(prog.b, prog.memoff);
        break_unmask_all(prog.b);
        break_activate_all(prog.b);
        prog.sdb_dumpaddr = -1;
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);
        if (!stop) {
            ptrace(PTRACE_CONT, child, 0, 0);
            waitpid(child, &status, 0);
            return tracee_stop_callback(status);
        }
    }
    
    return 0;
}