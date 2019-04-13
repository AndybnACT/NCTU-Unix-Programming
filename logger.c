#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// lock the PRELOAD_LOGGING_lock since startup routine may call
// injected functions 
int PRELOAD_LOGGING_lock = 1;
static FILE *logFILE;
static int  logFd;
static char PROCNAME[2048];
static char FILENAME[2048];

__attribute__((constructor))
static void startup(void) {
    char *outfile = getenv("MONITOR_OUTPUT");
    if (outfile) {
        logFILE = fopen(outfile, "w+");
        if (!logFILE) {
            printf("Error opening file\n");
            exit(EXIT_FAILURE);
        }
        logFd = fileno(logFILE);
    }else{
        // We need to duplicate the fd since program usually close stderr
        // of itself at the end. We won't be able to output our log if the
        // fd of that logging file is closed.
        int fd = fileno(stderr);
        logFd = dup(fd);
        logFILE = fdopen(logFd, "w+");
    }
    
    // unlock PRELOAD_LOGGING_lock so that any futher
    // injected function calls get logged
    PRELOAD_LOGGING_lock = 0;
}

__attribute__((destructor))
static void finish(void){
    // logging is no longer needed, lock it
    PRELOAD_LOGGING_lock = 1;
    fclose(logFILE);
}


int iterate_args(int ind, int cnt, struct argument *lst) {
    for (; ind < cnt; ind++) {
        // fprintf(logFILE, "%s:  ", lst[ind].type);
        lst[ind].selfprint(lst[ind].addr);
        if (ind < cnt-1) {
            fprintf(logFILE, ", ");
        }
    }
    return ind;
}

// some functions place return structure in argument list.
// Thus we explicitly find the function and then print the argument at exit by
// keepping track of the variable Ind
// ?? is there any better solution?
int Ind; // this global variable should be placed at TLS for multi-thread apps. 
void inject_log_entry(char* name, int count, struct argument *arg_list){
    
    Ind=0;
    if (strstr(name, "stat")) {
        
        count = 0;
    }
    fprintf(logFILE, "# %s( ", name);
    Ind = iterate_args(Ind, count, arg_list);
    
    return;
}
// We need to seperate entry and exit since some ptr may become invalid after
// injected calls are completed (like: fclose, free etc...)
void inject_log_exit(char* name, int count, struct argument *arg_list){
    Ind = iterate_args(Ind, count, arg_list);
    fprintf(logFILE, ") =  ");
    arg_list[count].selfprint(arg_list[count].addr);
    fprintf(logFILE, "\n");
    return;
}

// given a fd number returned from fileno or dirfd
// readlink the corresponding proc file and print filename of fd
void printfd(int fd){
    if (fd == -1)
        perror("error opening fd");
    snprintf(PROCNAME, 255, "/proc/self/fd/%d", fd);
    if (readlink(PROCNAME, FILENAME, 255) == -1){
        fprintf(logFILE, "readlink error: %s  ", PROCNAME);
    }else{
        fprintf(logFILE, "\"%s\"  ", FILENAME);
    }
    return;
}

// log printing routines go here
void log_TYPEP_stat(void *addr){
    struct stat *st;
    st = *((struct stat**)addr);
    fprintf(logFILE, "0x%lx {mode=0%o, size=%d}", st, (st->st_mode&0xFFF), st->st_size);
    return; 
}
void log_TYPEP_DIR(void *addr){
    DIR *dir = *((DIR**)addr);
    int fd;
    fd = dirfd(dir);
    printfd(fd);
    return;  
}
void log_TYPEP_dirent(void *addr){
    struct dirent *dir = *((struct dirent**) addr);
    if (dir) {
        fprintf(logFILE, "%s  ", dir->d_name);
    }else{
        fprintf(logFILE, "<NULL> ");
    }
    return;  
}
void log_TYPEP_char(void *addr){
    fprintf(logFILE, "\"%s\" ", *((char**)addr));
    return;
}
void log_TYPEP_FILE(void *addr) {
    FILE *fstream;
    int fd;
    fstream = *((FILE**)addr);
    if (fstream == stdout) {
        fprintf(logFILE, "<STDOUT>");
        return;
    }else if (fstream == stderr){
        fprintf(logFILE, "<STDERR>");
        return;
    }else if (fstream == stdin) {
        fprintf(logFILE, "<STDIN>");
        return;
    }
    fd = fileno(fstream);
    printfd(fd);
    return;
}
void log_TYPEP_void(void *addr){
    fprintf(logFILE, "0x%lx ", *((void**)addr) );
    return;  
}
void log_TYPE_fd(void *addr){
    int fd = *((int*)addr);
    printfd(fd);
    return;  
}
void log_TYPE_mode_t(void *addr){
    fprintf(logFILE, "0%o ", *((mode_t*)addr));
    return;  
}
void log_TYPE_ssize_t(void *addr){
    
    fprintf(logFILE, "%lld ", *((ssize_t*)addr));
    return;  
}
void log_TYPE_size_t(void *addr){
    fprintf(logFILE, "%zu ", *((size_t*)addr));
    return;
}
void log_TYPE_int(void *addr){
    fprintf(logFILE, "%d ", *((int*) addr) );
    return;  
}



