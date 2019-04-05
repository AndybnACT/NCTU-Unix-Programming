#include "select_net.h"
#include "netstat.h"
#include <regex.h> 


#define REGEXBUF 512
static char regexbuf[REGEXBUF];

#define regexErr(expr, preg)  do {                 \
    int rc;                                        \
    rc = (expr);                                   \
    if (rc) {                                      \
        regerror(rc, &(preg), regexbuf, REGEXBUF); \
        printf("%s\n", regexbuf);                  \
        exit(EXIT_FAILURE);                        \
    }                                              \
}while (0) 


int match(struct process *owner, regex_t checker){
    int regret;
    dprintf(4, "%p, %s", owner, owner->cmdline);
    regret = regexec(&checker, owner->cmdline, 0, NULL, 0);
    if (!regret) {
        dprintf(2, "\t\t[%s]  match!\n", owner->cmdline);
        return SHOW_THIS;
    }else if (regret == REG_NOMATCH){
        dprintf(2, "\t\t[%s]  no match\n", owner->cmdline);
        return !SHOW_THIS;
    }else{
        regerror(regret, &(checker), regexbuf, REGEXBUF); 
        printf("unexecpted error: %s\n", regexbuf);     
        exit(EXIT_FAILURE);             
    }

}


void select_net(char* pattern, struct net *head) {
    regex_t regex;
    if (pattern) {// construct regex
        regexErr( regcomp(&regex, pattern, REG_EXTENDED), regex );
    }else{ // no pattern, show all
        for (; head; head=head->next) {
            if (head->owner.pid > 0) 
                head->show = SHOW_THIS;
        }
        return;
    }
    
    for (; head; head=head->next) { // find match
        for (struct process *p = &(head->owner); p; p=p->next) {
            if (p->pid > 0)
                head->show |= match(p, regex);
        }
    }
    regfree(&regex);
    return;
}