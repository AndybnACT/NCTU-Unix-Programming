#include <stdio.h>

// #define DEBUG
#ifdef DEBUG
#define CONFIG_DEBUG_LEVEL 5

#define dprintf(lvl, fmt, args...){                             \
    if (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL) {    \
        printf((fmt), ##args);                                  \
    }                                                           \
}

#else

#define CONFIG_DEBUG_LEVEL 0
#define dprintf(lvl, fmt, args...) ;

#endif