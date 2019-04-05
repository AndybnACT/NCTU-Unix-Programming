#include "netstat.h"
#include "parse_net.h"
#include "result.h"
#include <stdio.h>
#include <arpa/inet.h>

#define TCP_BAR "\nList of TCP connections:\n"
#define UDP_BAR "\nList of UDP connections:\n"
#define TITLE_BAR "Proto Local Address           Foreign Address         PID/Program name and arguments\n"

static void title(struct net *cur) {
    static int protocol = 0;
    switch (cur->protocol) {
        case SHOW_TCP:
            if (protocol&SHOW_TCP) { // title has shown before
                return;
            }else{
                printf(TCP_BAR);
                printf(TITLE_BAR);
                protocol |= SHOW_TCP; // mark as showed
                return;
            }
        case SHOW_UDP:
            if (protocol&SHOW_UDP) { // title has shown before
                return;
            }else{
                printf(UDP_BAR);
                printf(TITLE_BAR);
                protocol |= SHOW_UDP; // mark as showed
                return;
            }
        default:
            printf("error printing title\n");
    }
}

static void proto(struct net *cur){
    switch (cur->protocol) {
        case SHOW_TCP:
            printf("tcp");
            break;
        case SHOW_UDP:
            printf("udp");
            break;
        default:
            printf("error printing proto\n");
            return;
    }
    if (cur->version == AF_INET6) {
        printf("6  ");
    }else{
        printf("   ");
    }
}

static void ips(struct net* cur){
    char buf[255];
    if (!snprintf(buf, 255, "%s:%ld",cur->local.ip_string, cur->local.port)) {
        perror("error generating cmdline file name");
        exit(EXIT_FAILURE);
    }
    printf("%-24s", buf);
    if (!snprintf(buf, 255, "%s:%ld",cur->remote.ip_string, cur->remote.port)) {
        perror("error generating cmdline file name");
        exit(EXIT_FAILURE);
    }
    printf("%-24s", buf);
}

static void cmd(struct net* cur) {
    struct process *p = &(cur->owner);
    for (; p; p=p->next) {
        printf(";%d/%s     ", p->pid, p->cmdline);
    }
    printf("\n");
}


void show_result(struct net *head) {
    for (struct net *cur = head->next; cur; cur=cur->next) {
        if (cur->show & SHOW_THIS) {
                title(cur); // title
                proto(cur); // protocol
                ips(cur);   // ip
                cmd(cur);   // cmdline
        }    
    }
    return;
}