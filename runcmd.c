#include <stdio.h>
#include <string.h>
#include "runcmd.h"
#include "debug.h"

int dbg_exit(int argc, char **argv);
int help(int argc, char **argv);

struct command Commands[] = {
    {"help"  , "h",  "- help: show this message", help},
    {"break" , "b",  "- break {instruction-address}: add a break point", sdb_break},
    {"list"  , "l",  "- list: list break points", sdb_listb},
    {"delete", NULL, "- delete {break-point-id}: remove a break point", sdb_delb},
    {"vmmap" , "m",  "- vmmap: show memory layout", vmmap},
    {"run"   , "r",  "- run: run the program", sdb_run},
    {"cont"  , "c",  "- cont: continue execution", sdb_cont},
    {"dump"  , "x",  "- dump addr [length]: dump memory content", sdb_dump},
    {"disasm", "d",  "- disasm addr: disassemble instructions in a file or a memory region", dis_asm},
    {"start" , NULL, "- start: start the program and stop at the first instruction", sdb_start},
    {"si"    , NULL, "- si: step into instruction", sdb_si},
    {"get"   , "g",  "- get reg: get a single value from a register", sdb_getreg},
    {"set"   , "s",  "- set reg val: get a single value to a register", sdb_setreg},
    {"getregs", NULL,"- getregs: show registers", sdb_getregs },
    {"load"  ,  NULL,"- load {path/to/a/program}: load a program", load_prog},
    {"exit"  , "q",  "- exit: terminate the debugger", dbg_exit}
};
const int NCOMMAND = (sizeof(Commands)/sizeof(struct command));

typedef int (*_func)(int, char**);
_func find_func(char *cmd){
    int i = 0;

    for (i = 0; i < NCOMMAND; i++) {
        if (strcmp(cmd, Commands[i].name) == 0) {
            return Commands[i].func;
        }else if (Commands[i].shortcut) {
            if (strcmp(Commands[i].shortcut, cmd) == 0)
                return Commands[i].func;
        }
    }
    return NULL;
}


#define DELIMITER "\n\t\r "
#define MAXARGS 10
int runcmd(char *buf){
    int argc = 0;
    char *argv[MAXARGS];
    int (*cmd)(int, char**);
    
    while (1) {
        while (strchr(DELIMITER, *buf) && *buf) {
            *buf = '\0';
            buf++;
        }

        if (*buf == '\0')
            break;

        if (argc >= MAXARGS) {
            dprintf(0, "argument exceed MAXARGS\n");
            return -1;
        }
        argv[argc++] = buf;
        
        while(!strchr(DELIMITER, *buf) && *buf)
            buf++;
        
    }
    for (int i = 0; i < argc; i++) {
        dprintf(0, "argv[%d] = %s\n", i, argv[i]);
    }
    
    if (argc == 0)
        return 0;
    
    cmd = find_func(argv[0]);
    if (cmd) {
        return cmd(argc, argv);
    }else{
        printf("Command not found!\n");
        return 0;
    }
}

int help(int argc, char **argv){
    printf("Command Usage:\n");
    for (size_t i = 0; i < NCOMMAND; i++) {
        printf("%s\n", Commands[i].help);
    }
    return 0;
}

int dbg_exit(int argc, char **argv){
    return -1;
}

