#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "debug.h"
#include "regs.h"

unsigned long long str2num(char *str){
    char *endptr;
    unsigned long long ret;
    errno = 0;
    if (!(*str)) {
        printf("** cannot convert null string to number\n");
        return -1;
    }
    ret = strtoull(str, &endptr, 0);
    if (*endptr) {
        printf("** warning str2num conversion not completed\n");
        if (!errno) errno = EINVAL;
        return -1;
    }
    if (errno) {
        perror("strtoull ");
        return -1;
    }
    return ret;
    
}

void dump_all_regs(struct user_regs_struct *regs){
    size_t i;
    for (i = 0; i < NREGS; i++) {
        printf("%s = %lld (%llx)\n",
               REGS[i].name, REGS[i].get(regs), REGS[i].get(regs));
    }
}

int dump_reg(struct user_regs_struct *regs, char *regname){
    size_t i;
    for (i = 0; i < NREGS; i++) {
        if (strcmp(regname, REGS[i].name) == 0) {
            printf("%s = %lld (%llx)\n", 
                   REGS[i].name, REGS[i].get(regs), REGS[i].get(regs));
            goto success;
        }
    }
//failed:
    printf("** warning, register '%s' not found\n", regname);
    return -1;
success:
    return 0;
}

int set_reg(struct user_regs_struct *regs, char *regname, unsigned long long val){
    size_t i;
    for (i = 0; i < NREGS; i++) {
        if (strcmp(regname, REGS[i].name) == 0) {
            REGS[i].set(regs, val);
            goto success;
        }
    }
//failed:
    printf("** warning, register '%s' not found\n", regname);
    return -1;
success:
    return 0;
}


void dump_hex(char *str, long long start, long long size){
    int wcnt = 0;
    long long addr = start;
    while (wcnt < size) {
        
        if (!(wcnt % 16)){
            printf("%08llx: ", addr);
            addr += 16;
        }
        if (!(wcnt % 8)) {
            // seperate 8-byte wide
            printf(" ");
        }
        
        printf("%02hhx ",str[wcnt]);
        
        if (wcnt % 16 == 15) {
            printf("|");
            for (size_t i = wcnt-15; i <= wcnt; i++) {
                printf("%c", (isgraph(str[i])) ? str[i]:'.');
            }
            printf("|\n");
        }    
        wcnt++;
    }
    if (wcnt % 16) {
        printf("|");
        for (size_t i = wcnt-(wcnt%16); i < wcnt; i++) {
            printf("%c", (isgraph(str[i])) ? str[i]:'.');
        }
        printf("|\n");
    }
}


int read_mapping (FILE *mapfile, 
	      unsigned long long *addr, 
	      unsigned long long *endaddr, 
	      char *permissions, 
	      unsigned long long *offset, 
	      char *device, 
	      unsigned long long *inode, 
	      char *filename)
{
  int ret = fscanf (mapfile,  "%llx-%llx %s %llx %s %llx", 
		    addr, endaddr, permissions, offset, device, inode);

  if (ret > 0 && ret != EOF && *inode != 0)
    {
      ret += fscanf (mapfile, "%s\n", filename);
    }
  else
    {
      char buf[255];
      filename[0] = '\0';	/* no filename */
      fgets(buf, 255, mapfile);
      sscanf(buf, "%s\n", filename);
    }
  return (ret != 0 && ret != EOF);
}