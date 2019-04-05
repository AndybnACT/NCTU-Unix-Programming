#ifndef NETSTAT
#define NETSTAT

#include <stdio.h>
#include <stdlib.h>

// #define CONFIG_DEBUG
// #define CONFIG_DEBUG_IP_PARSER

#ifdef CONFIG_DEBUG
    #define CONFIG_DEBUG_LEVEL 2
    #ifdef CONFIG_DEBUG_IP_PARSER
        #define PROC_TCP  "./iptest/tcp"
        #define PROC_TCP6 "./iptest/tcp6"
        #define PROC_UDP  "./iptest/udp"
        #define PROC_UDP6 "./iptest/udp6"
    #else
        #define PROC_TCP  "/proc/net/tcp"
        #define PROC_TCP6 "/proc/net/tcp6"
        #define PROC_UDP  "/proc/net/udp"
        #define PROC_UDP6 "/proc/net/udp6"
    #endif
#else
    #define CONFIG_DEBUG_LEVEL 0
    #define PROC_TCP  "/proc/net/tcp"
    #define PROC_TCP6 "/proc/net/tcp6"
    #define PROC_UDP  "/proc/net/udp"
    #define PROC_UDP6 "/proc/net/udp6"
    
#endif

#define PROC_ROOT "/proc"


#define SHOW_TCP 0x0001
#define SHOW_UDP 0x0002
#define SHOW_THIS 0x1

#define dprintf(lvl, fmt, args...) do {                         \
        if (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
               printf((fmt) , ##args );                         \
    } while (0)
    
#endif