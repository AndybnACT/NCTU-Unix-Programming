#include "common.h"
#include <stdio.h>
#include <string.h>

// Given a function name, funcname, find the varient function that accept
// variadic list as last parameter
void* find_vfmtfunc(char *funcname){
    if (strstr(funcname, "fscanf")) {
        return (void*) dlsym(RTLD_NEXT, "vfscanf");
    }else if (strstr(funcname, "fprintf")) {
        return (void*) dlsym(RTLD_NEXT, "vfprintf");
    }else if (strstr(funcname, "printf")) {
        return (void*) dlsym(RTLD_NEXT, "vprintf");
    }else if (strstr(funcname, "scanf")) {
        return (void*) dlsym(RTLD_NEXT, "vscanf");
    }
}