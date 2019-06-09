#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "runcmd.h"
#include "debug.h"

int STATE = STATE_NONE; 
struct prog_ctx prog = {0, };

int state_cleanup(void){
    if (STATE) {
        dprintf(0, "clenup function not implemented!\n");
    }
    if (STATE & STATE_LOAD) {
        /* code */
    }
    if (STATE & STATE_RUNNING) {
        /* code */
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    char *cmdbuf = NULL;
    unsigned long bufsize = 0;
    int ret = -1;
    if (argc == 2) {
        dprintf(0, "argc = 2, not implemented!\n");
    }else if (argc != 1) {
        printf("Usage: ./sdb [program to debug]\n");
        goto bad_exit;
    }
    
    
    while (1) {
    // get input command  
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