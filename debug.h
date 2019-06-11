#include <stdio.h>
#define CONFIG_DEBUG_LEVEL 5

#define dprintf(lvl, fmt, args...){                             \
    if (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL) {    \
        printf((fmt), ##args);                                  \
    }                                                           \
}
