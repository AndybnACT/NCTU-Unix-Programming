#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include "debug.h"
#include "util.h"
#include "runcmd.h"

int tracee_startup(unsigned long);
int tracee_cont(void);
int tracee_getallregs(void);
int tracee_setallregs(void);

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
    
    // rollback current breakpoint if exist
    dprintf(0, "cont_tracee not fully implemented!\n");
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
    if (val == -1) {
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



int tracee_stop_callback(int status){
    if (WIFEXITED(status)) {
        int exitcode = WEXITSTATUS(status);
        printf("** child process %s terminated %s (code %d)\n",
               prog.progname, 
               (exitcode == 0)? "normally":"abnormally",
               exitcode);
        STATE &= ~STATE_RUNNING;
        prog.pid = -1;
        return 0;
    }
    dprintf(0, "tracee_stop_callback not implemented!\n");
    return 0;
}

int tracee_cont(void){
    pid_t child = prog.pid;
    int status;
    ptrace(PTRACE_CONT, child, 0, 0);
    if (waitpid(child, &status, 0) < 0) {
        perror("waitpid ");
    }
    return tracee_stop_callback(status);
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
        assert(WIFSTOPPED(status));
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);
        if (!stop) {
            ptrace(PTRACE_CONT, child, 0, 0);
            waitpid(child, &status, 0);
            return tracee_stop_callback(status);
        }
    }
    
    return 0;
}