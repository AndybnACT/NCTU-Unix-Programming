#include "common.h"
#include "va_macro.h"
#include <stdarg.h>
#include <sys/types.h>

typedef struct FILE*    TYPEP_FILE;   
typedef struct stat*    TYPEP_stat;
typedef struct DIR*     TYPEP_DIR;
typedef struct dirent*  TYPEP_dirent;
typedef char*           TYPEP_char;
typedef void*           TYPEP_void;
typedef mode_t    TYPE_mode_t;
typedef ssize_t   TYPE_ssize_t;
typedef size_t    TYPE_size_t;
typedef int       TYPE_int;
typedef int       TYPE_fd;

void inject_log_entry(char* , int, struct argument *);
void inject_log_exit(char* , int, struct argument *);
#define INJECT_DEFINE(ret_type, func, num, ...)                     \
ret_type func(VA_CONCAT_DECL_##num(__VA_ARGS__)){                   \
    ret_type (*real_func)(__VA_ARGS__);                             \
    ret_type retval;                                                \
    struct argument args[num+2]={{0,0},};                           \
    real_func = dlsym(RTLD_NEXT, #func);                            \
    if (!PRELOAD_LOGGING_lock) {                                    \
        PRELOAD_LOGGING_lock = 1;                                   \
        VA_MAP_UNROLL_##num(args, __VA_ARGS__);                     \
        inject_log_entry(#func, num, args);                         \
        PRELOAD_LOGGING_lock = 0;                                   \
    }                                                               \
    retval = real_func(VA_CONCAT_##num());                          \
    if (!PRELOAD_LOGGING_lock) {                                    \
        PRELOAD_LOGGING_lock = 1;                                   \
        args[num].type = #ret_type;                                 \
        args[num].addr = &retval;                                   \
        args[num].selfprint = log_##ret_type;                       \
        inject_log_exit(#func, num, args);                          \
        PRELOAD_LOGGING_lock = 0;                                   \
    }                                                               \
    return retval;                                                  \
}

// These routine is only usable when there exists a 'v'ariadic variant function 
// like vfprintf -> fprintf
// We manually postpone the inject_log_entry here to wait for completion of real_func
// since our log may be intercepted by output of printing functions like `fprintf`
// #----> better solution would be adding a option at INJECT_DEFINE() to decide
// #----> whether to output logging at the entry or exit
void* find_vfmtfunc(char *funcname);
#define INJECT_DEFINE_FORMAT_FUNC(ret_type, func, num_must, ...)                \
ret_type func(VA_CONCAT_DECL_##num_must(__VA_ARGS__), ...){                     \
    ret_type (*real_func)(__VA_ARGS__, ...);                                    \
    ret_type retval;                                                            \
    struct argument args[num_must+2]={{0,0},};                                  \
    va_list va;                                                                 \
    va_start(va, op##num_must);                                                 \
    if (!PRELOAD_LOGGING_lock) {                                                \
        PRELOAD_LOGGING_lock = 1;                                               \
        VA_MAP_UNROLL_##num_must(args, __VA_ARGS__);                            \
        PRELOAD_LOGGING_lock = 0;                                               \
    }                                                                           \
    if (!PRELOAD_LOGGING_lock){                                                 \
        PRELOAD_LOGGING_lock = 1;                                               \
        real_func = (ret_type (*)(__VA_ARGS__, ...)) find_vfmtfunc(#func);      \
        PRELOAD_LOGGING_lock = 0;                                               \
    }else{                                                                      \
        real_func = (ret_type (*)(__VA_ARGS__, ...)) find_vfmtfunc(#func);      \
    }                                                                           \
    retval = real_func(VA_CONCAT_##num_must(), va);                             \
    if (!PRELOAD_LOGGING_lock) {                                                \
        PRELOAD_LOGGING_lock = 1;                                               \
        args[num_must].type = #ret_type;                                        \
        args[num_must].addr = &retval;                                          \
        args[num_must].selfprint = log_##ret_type;                              \
        inject_log_entry(#func, num_must, args);                                \
        inject_log_exit(#func, num_must, args);                                 \
        PRELOAD_LOGGING_lock = 0;                                               \
    }                                                                           \
    va_end(va);                                                                 \
    return retval;                                                              \
}

INJECT_DEFINE(TYPE_int,      closedir, 1, TYPEP_DIR)
INJECT_DEFINE(TYPEP_DIR,     opendir, 1, TYPEP_char)
INJECT_DEFINE(TYPEP_dirent,  readdir, 1, TYPEP_DIR)
INJECT_DEFINE(TYPE_int,      creat, 2, TYPEP_char, TYPE_mode_t)
INJECT_DEFINE(TYPE_int,      open, 2, TYPEP_char, TYPE_mode_t)
INJECT_DEFINE(TYPE_ssize_t,  read, 3, TYPE_int, TYPEP_void, TYPE_size_t)
INJECT_DEFINE(TYPE_ssize_t,  write, 3, TYPE_int, TYPEP_void, TYPE_size_t)
INJECT_DEFINE(TYPE_int,      dup, 1, TYPE_int)
INJECT_DEFINE(TYPE_int,      dup2, 2, TYPE_int, TYPE_int)
INJECT_DEFINE(TYPE_int,      close, 1, TYPE_fd)
INJECT_DEFINE(TYPE_int,      __xstat, 3, TYPE_int, TYPEP_char, TYPEP_stat)
INJECT_DEFINE(TYPE_int,      __lxstat, 3, TYPE_int, TYPEP_char, TYPEP_stat)
INJECT_DEFINE(TYPE_ssize_t,  pwrite, 4, TYPE_int, TYPEP_void, TYPE_size_t, TYPE_int)
INJECT_DEFINE(TYPE_ssize_t,  pread, 4, TYPE_int, TYPEP_void, TYPE_size_t, TYPE_int)
INJECT_DEFINE(TYPEP_void,    fopen, 2, TYPEP_char, TYPEP_char)
INJECT_DEFINE(TYPE_int,      fclose, 1, TYPEP_FILE)
INJECT_DEFINE(TYPE_size_t,   fread, 4, TYPEP_void, TYPE_size_t, TYPE_size_t, TYPEP_FILE)
INJECT_DEFINE(TYPE_size_t,   fwrite, 4, TYPEP_void, TYPE_size_t, TYPE_size_t, TYPEP_FILE)
INJECT_DEFINE(TYPE_int,      fgetc, 1, TYPEP_FILE)
INJECT_DEFINE(TYPEP_char,    fgets, 3, TYPEP_char, TYPE_int, TYPEP_FILE)
INJECT_DEFINE_FORMAT_FUNC(TYPE_int,      fscanf, 2, TYPEP_FILE, TYPEP_char)
INJECT_DEFINE_FORMAT_FUNC(TYPE_int,      __isoc99_fscanf, 2, TYPEP_FILE, TYPEP_char)
INJECT_DEFINE_FORMAT_FUNC(TYPE_int,      fprintf, 2, TYPEP_FILE, TYPEP_char)
INJECT_DEFINE(TYPE_int,      chdir, 1, TYPEP_char);
INJECT_DEFINE(TYPE_int,      chown, 3, TYPEP_char, TYPE_int, TYPE_int);
INJECT_DEFINE(TYPE_int,      chmod, 2, TYPEP_char, TYPE_mode_t);
INJECT_DEFINE(TYPE_int,      remove, 1, TYPEP_char);
INJECT_DEFINE(TYPE_int,      rename, 2, TYPEP_char, TYPEP_char);
INJECT_DEFINE(TYPE_int,      link, 2, TYPEP_char, TYPEP_char);
INJECT_DEFINE(TYPE_int,      unlink, 1, TYPEP_char);
INJECT_DEFINE(TYPE_ssize_t,  readlink, 3, TYPEP_char, TYPEP_char, TYPE_size_t);
INJECT_DEFINE(TYPE_int,      symlink, 2, TYPEP_char, TYPEP_char);
INJECT_DEFINE(TYPE_int,      mkdir, 2, TYPEP_char, TYPE_mode_t);
INJECT_DEFINE(TYPE_int,      rmdir, 1, TYPEP_char);
// functions not mentioned on the spec
INJECT_DEFINE(TYPE_int,      fflush, 1, TYPEP_FILE);
INJECT_DEFINE(TYPE_int,      fputs_unlocked, 2, TYPEP_void, TYPEP_FILE);
INJECT_DEFINE(TYPE_int,      fwrite_unlocked, 4, TYPEP_void, TYPE_size_t, TYPE_size_t, TYPEP_FILE);
