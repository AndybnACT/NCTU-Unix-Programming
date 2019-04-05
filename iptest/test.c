#include <stdio.h>
#include <arpa/inet.h>
int main(int argc, char const *argv[]) {
    // IPv6 demo of inet_ntop() and inet_pton()
    // (basically the same except with a bunch of 6s thrown around)

    struct sockaddr_in6 sa;
    char str[INET6_ADDRSTRLEN];

    // store this IP address in sa:
    inet_pton(AF_INET6, "2001:db8:8714:3a90::12", &(sa.sin6_addr));
    printf("%llx %llx %llx %llx\n",sa.sin6_addr.s6_addr32[0],
           sa.sin6_addr.s6_addr32[1],
           sa.sin6_addr.s6_addr32[2],
           sa.sin6_addr.s6_addr32[3] );
    // now get it back and print it
    inet_ntop(AF_INET6, &(sa.sin6_addr), str, INET6_ADDRSTRLEN);

    printf("%s\n", str); // prints "2001:db8:8714:3a90::12"
    return 0;
}