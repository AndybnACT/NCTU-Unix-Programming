#define _GNU_SOURCE // --> it should be at the first line though I dont know why
#include <dlfcn.h>  // functions resolver

struct argument{
    char *type;
    void *addr;
    void (*selfprint)(void*);
};

// We need a lock to prevent recursive logging,
//     a.k.a, calling any injected functions in inject_log().
// The var should be placed on thread local storage 
// on multi-thread applications. 
extern int PRELOAD_LOGGING_lock;

void log_TYPEP_stat(void *);
void log_TYPEP_DIR(void *);
void log_TYPEP_dirent(void *);
void log_TYPEP_char(void *);
void log_TYPEP_FILE(void *);
void log_TYPEP_void(void *);
void log_TYPE_fd(void *);
void log_TYPEP_void(void *);
void log_TYPE_mode_t(void *);
void log_TYPE_ssize_t(void *);
void log_TYPE_size_t(void *);
void log_TYPE_int(void *);

// Development notes:
// To add a new injected funtion (with fix argument list (number of argument<9)),
// please follow the instruction below:
// 1. INJECT_DEFINE() the function, type of argmuments and return value should
//    be prefix with TYPE or TYPEP if it is a pointer.
//    INJECT_DEFINE([return type], )
// 2. typedef the type at fsmon.c if there is a new type.
// 3. if new type is defined, please define and implement the corresponding
//    logging routines at common.h and logger.c
// * Do not include header file of injected function in fsmon.c or you will
//   get name conflict!
 