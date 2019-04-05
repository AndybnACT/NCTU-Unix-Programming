#ifndef PARSE_NET
#define PARSE_NET
// regex
// #include <sys/types.h>
// #include <regex.h>  

struct address {

    unsigned long port;
    char *ip_string;
};
struct process {
    char *cmdline;
    int pid;
    struct process *next;
};
struct net {
    unsigned int version;
    struct address local;
    struct address remote;
    unsigned long inode;
    unsigned int protocol;
    // int pid;
    // char *cmdline;
    int nrowner;
    struct process owner;
    struct net *next;
    int show;
};

void parse_net(int, struct net *);

// #define LINEBUF 512

// 

#endif
