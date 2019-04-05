#include "parse_net.h"
#include "netstat.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
// address transform
#include <netinet/in.h>
#include <arpa/inet.h>

static struct net* file2net(FILE*, struct net*, unsigned int, unsigned int);

void parse_net(int flags, struct net *head) {
    while (flags) {
        FILE *v4fp, *v6fp;
        if (flags & SHOW_TCP) {
            // open proc file for reading
            v4fp = fopen(PROC_TCP, "r");
            v6fp = fopen(PROC_TCP6, "r");
            
            head = file2net(v4fp, head, AF_INET, SHOW_TCP);
            // append v6 connection to the end of v4's.
            head = file2net(v6fp, head, AF_INET6, SHOW_TCP);
            
            flags &= ~SHOW_TCP;
        }else if (flags & SHOW_UDP) {
            // open proc file for reading
            v4fp = fopen(PROC_UDP, "r");
            v6fp = fopen(PROC_UDP6, "r");
            
            head = file2net(v4fp, head, AF_INET, SHOW_UDP);
            // append v6 connection to the end of v4's.
            head = file2net(v6fp, head, AF_INET6, SHOW_UDP);
            
            flags &= ~SHOW_UDP;
        }        
        fclose(v4fp);
        fclose(v6fp);
    }
    return;
}

void ip_string(char* start, unsigned int version, struct address* addr_struct){
    char *ipstr;
    uint32_t ip[4] = {0, };
    char addr_buf[9] = {'\0',};
    
    dprintf(5, "\t\t\t\t reading raw address: ");
    for (int i = 0; i < ((version==AF_INET6)?4:1); i++) {
        memcpy(addr_buf, start, 8);
        ip[i] = (uint32_t) strtoul(addr_buf, NULL, 16);
        dprintf(5, "%llx\t", ip[i]);
        start += 8;
    }
    dprintf(5,"\n");
    addr_struct->port = (uint32_t) strtoul(start+1, NULL, 16);
    
    if (!(version & ~AF_INET)) {
        ipstr = (char*) malloc(INET_ADDRSTRLEN); // allocate string
        inet_ntop(AF_INET, ip, ipstr, INET_ADDRSTRLEN);
        
        addr_struct->ip_string = ipstr;
        
    }else if(!(version & ~AF_INET6)){
        ipstr = (char*) malloc(INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, ip, ipstr, INET6_ADDRSTRLEN);
        
        addr_struct->ip_string = ipstr;
        
    }else{
        printf("ip version unrecognized\n");
        exit(EXIT_FAILURE);
    }
    
    // addr_struct->port =  strtoul(remain+1, NULL, 16);
    dprintf(4, "\t\t\traw_ip=0x%p%p%p%p:0x%lx\n", 
           ip[0], ip[1], ip[2], ip[3], addr_struct->port);    
    return;
}

struct net* file2net(FILE *fp, struct net *head, unsigned int version, 
                     unsigned int session) {
    char linebuf[512];
    char *match;
    unsigned int off_local, off_remote, off_inode;
    
    dprintf(1, "reading proc file (%s,%s)\n", 
            version==AF_INET?"ipv4":"ipv6",
            session==SHOW_TCP?"tcp":"udp");
    // get first line of file
    fgets(linebuf, 512, fp);
    // get offset of file
    match = strstr(linebuf, "local");
    off_local = match - linebuf;
    match = strstr(linebuf, "rem");
    off_remote = match - linebuf;
    match = strstr(linebuf, "inode");
    off_inode = match - linebuf;
    dprintf(2, "\toffset = [%d,%d,%d]\n", off_local, off_remote, off_inode);
    
    while (fgets(linebuf, 512, fp)) { // get lines
        struct net *node;
        node = (struct net*) malloc(sizeof(struct net));
        head->next = node;
        dprintf(3, "\t\tparsing version = %s\n", (version==AF_INET6?"ipv6":"ipv4"));
        ip_string(linebuf+off_local, version, &(node->local));
        ip_string(linebuf+off_remote, version, &(node->remote));
        node->inode = strtol(linebuf+off_inode, NULL, 10);
        node->protocol = session;
        node->next = NULL;
        node->version = version;
        node->nrowner = 0;
        node->owner.pid = -1;
        node->owner.next = NULL;
        node->show = !SHOW_THIS;
        head = node;
        dprintf(3,"\t\tparse complete: local:%s\tremote:%s\tinode:%ld\n", 
                node->local.ip_string, node->remote.ip_string, node->inode);
    }
    return head;
}