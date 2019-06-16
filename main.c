#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "runcmd.h"
#include "debug.h"

int STATE = STATE_NONE; 
struct prog_ctx prog = {0, };

int state_cleanup(void){
    if (STATE) {
        dprintf(10, "clenup function not fully implemented!\n");
    }
    if (STATE & STATE_LOAD) {
        close(prog.fd);
        free(prog.progname);
        free(prog.asm_file.data);
    }
    if (STATE & STATE_RUNNING) {
        /* code */
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *cmdbuf = NULL;
    unsigned long bufsize = 0;
    int ret = -1;
    if (argc == 2) {
        load_prog(2, argv);
    }else if (argc != 1) {
        printf("Usage: ./sdb [program to debug]\n");
        goto bad_exit;
    }
    
    
    while (1) {
    // get input command  
        printf("sdb> ");
        errno = 0;
        ret = getline(&cmdbuf, &bufsize, stdin);
        if (ret == -1) {
            if (errno) {
                perror("getline");
                goto state_exit;
            }
            goto free_exit;
        }
    // call the parsing routine and handle command
        if (ret && *cmdbuf != '\n') {
            dprintf(0, "%d characters read: %s", ret, cmdbuf);
            if ((ret = runcmd(cmdbuf)) == -1)
                goto free_exit;
        }
    }

free_exit:
    free(cmdbuf);
state_exit:
    ret = state_cleanup();
bad_exit:
    return ret;
}