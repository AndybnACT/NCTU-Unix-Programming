#include "netstat.h"
#include "parse_net.h"
#include "search.h"
#include "select_net.h"
#include "result.h"
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>

int main(int argc, char* const *argv) {
    int netstat_flag = 0;
    struct net *socket_list;
    // parse cmd-line
    while (1) {
        static struct option long_options[]={
            {"tcp", no_argument, NULL, 't'},
            {"udp", no_argument, NULL, 'u'},
            {0,0,0,0}
        };
        int opt, opt_inx;
        opt = getopt_long(argc, argv, "tu", long_options, &opt_inx);
        if (opt == -1)
            break;
        switch (opt) {
            case 't':
                netstat_flag |= SHOW_TCP;
                break;
            case 'u':
                netstat_flag |= SHOW_UDP;
                break;
            case '?':
                printf("option not recognized\n");
                exit(EXIT_FAILURE);
            case 0:
                printf("bug, flag is not null\n");
                exit(EXIT_FAILURE);
            default:
                // not reach
                printf("bug, opt=%d\n", opt);
                break;
        }
    }
    if (!netstat_flag) // default
        netstat_flag = SHOW_TCP|SHOW_UDP;
    argc -= optind;
    argv += optind;
    dprintf(1, "parse complete flag=%d, remaining: %s\n",netstat_flag, argv[0]);
    
    
    // parse /proc/net/tcp & tcp6
    // parse /proc/net/udp & udp6
    socket_list = (struct net*) malloc(sizeof(struct net));
    socket_list->show = !SHOW_THIS;
    parse_net(netstat_flag, socket_list);
    for (struct net *cur = socket_list->next; cur; cur=cur->next) {
        dprintf(1, "prot: %s\t version: %s\t local_ip: %s:%d\t rem_ip:%s:%d\t inode=%d\n",
                (cur->protocol==SHOW_TCP?"tcp":"udp"), 
                (cur->version==AF_INET?"ipv4":"ipv6"),
                cur->local.ip_string, cur->local.port,
                cur->remote.ip_string, cur->remote.port,
                cur->inode);
    }
    
    
    // search in /proc/[pid]/fd
    search_process(socket_list);
    for (struct net *cur = socket_list->next; cur; cur=cur->next) {
        dprintf(1, "prot: %s\t version: %s\t local_ip: %s:%d\t rem_ip:%s:%d\t inode=%d\t",
                (cur->protocol==SHOW_TCP?"tcp":"udp"), 
                (cur->version==AF_INET?"ipv4":"ipv6"),
                cur->local.ip_string, cur->local.port,
                cur->remote.ip_string, cur->remote.port,
                cur->inode);
        for (struct process *p=&(cur->owner); p; p=p->next) {
            dprintf(1, "pid=%d\t cmdline=%s\t", p->pid, p->cmdline);
        }
        dprintf(1,"\n");
    }
    
    
    // construct regex and perform searching
    select_net(argv[0], socket_list);
    
    // show result
    show_result(socket_list);
    
    return 0;
}