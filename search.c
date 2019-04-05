#include "search.h"
#include "netstat.h"

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>

static void iterate_fd(char *, int, DIR*, struct net*);

void search_process(struct net *socket_list){
    DIR *proc;
    struct dirent *dp;
    int pidnr;
    proc = opendir(PROC_ROOT);
    if (!proc) {
        perror("error opening /proc");
        exit(EXIT_FAILURE);
    }
    // open dir '/proc/pid/fd'
    while ((dp = readdir(proc))) {
        if (dp->d_type == DT_DIR && (pidnr = atoi(dp->d_name))) {// is a [pid] directory
            char fdname[512];
            if (!snprintf(fdname, 255,"%s/%s/fd", PROC_ROOT, dp->d_name)) {
                perror("error generating path");
                exit(EXIT_FAILURE);
            }


    // read link(or stat) of fds 
            dprintf(5, "\t\topening %s\n", fdname);
            DIR *pid_fd;
            if (!(pid_fd = opendir(fdname))){
                perror("error opening proc/pid/fd");
                exit(EXIT_FAILURE);
            };
            iterate_fd(fdname, pidnr, pid_fd, socket_list);
            closedir(pid_fd);
            
        }
    }
    closedir(proc);
    return;
}


static struct net* sock_match_inode(unsigned long inode, struct net* head){
    for (struct net *cur = head; cur; cur=cur->next) {
        if (cur->inode == inode)
            return cur;
    }
    return NULL;
}

unsigned long fdfile_get_inode(char* fdfilename){
    char buf[255];
    int strlenth;
    unsigned long ret;
    if ((strlenth = readlink(fdfilename, buf, 255)) == -1) {
        perror("error reading link");
        exit(EXIT_FAILURE);
    }
    buf[strlenth] = '\0';
    ret = strtoul(buf+SOCK_OFFSET, NULL, 10);
    return ret;
}

char *get_cmdline(int pid){
    char buf[255];
    char *ret;
    int fd, len;//readlen=0, totallen=0;
    if (!snprintf(buf, 255, "%s/%d/cmdline", PROC_ROOT, pid)) {
        perror("error generating cmdline file name");
        exit(EXIT_FAILURE);
    }
    // open file
    if (!(fd = open(buf, O_RDONLY))){
        printf("cannot open %s\n", buf);
        perror("error opening cmdline file");
        exit(EXIT_FAILURE);
    }

    // while ((readlen = read(fd, buf+totallen, 255-totallen)) != -1) {
    //     buf[readlen+totallen] = ' ';
    //     totallen = totallen + 1 + readlen;
    // }
    if ((len = read(fd, buf, 255)) == -1){
        perror("error reading cmdline");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < len-1; i++) {
        if (buf[i] == '\0') 
            buf[i] = ' ';
    }
    dprintf(4, "\t\t\t\tbuf=%s\n", buf);
    ret = (char*) malloc(len);
    memcpy(ret, buf, len);
    close(fd);
    return ret;
}

void net_append_owner(struct net *net, struct process *buf) {
    struct process *lastowner;
    if (net->nrowner) {
        lastowner = &(net->owner);
        for (;lastowner->next; lastowner=lastowner->next);
        lastowner->next = (struct process*) malloc(sizeof(struct process));
        memcpy(lastowner->next, buf, sizeof(struct process));
    }else{
        memcpy(&(net->owner), buf, sizeof(struct process));
    }
    net->nrowner++;
    return;
}

static void iterate_fd(char *curdir_name, int nr, DIR* dir, struct net *list){
    struct dirent *dp;
    struct net *found;
    struct process owner_buf;
    char filename_buf[512] = {'\0', };
    // now, we're in /proc/[pid]/fd/
    while ((dp = readdir(dir))) {
        if (!snprintf(filename_buf, 255, "%s/%s", curdir_name, dp->d_name)) {
            perror("error generating fd file name");
            exit(EXIT_FAILURE);
        }
        
    // read link of each fd file
        struct stat status;
        if (stat(filename_buf, &status)){
            perror("cannot stat");
            printf("%s\n", filename_buf);
            exit(EXIT_FAILURE);
        };
        
    // if fd is a socket, pares out socket number
        if (S_ISSOCK(status.st_mode)) {
            unsigned long inode;
            inode = fdfile_get_inode(filename_buf);
            dprintf(5, "\t\t\t\tinode: %ld @ %s is a socket\n", inode,
                    filename_buf);

    // find match in the list and get cmdline
            if ((found = sock_match_inode(inode, list))){
                owner_buf.pid = nr;
                owner_buf.next = NULL;
                owner_buf.cmdline = get_cmdline(nr);
                net_append_owner(found, &owner_buf);
                dprintf(4, "\t\t\t\tinode=%ld, pid=%d, cmdline=%s\n", inode, 
                        nr, owner_buf.cmdline);
            }else{
                dprintf(5, "\t\t\t\tinode=%ld not found in socket list\n", inode);
            }

        }
        
    }
    
    return;
}


