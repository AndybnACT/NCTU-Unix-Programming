#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "runcmd.h"
#include "elftool.h"


int load_prog(int argc, char **argv){
    int i;
    elf_handle_t *eh = NULL;
    elf_strtab_t *tab = NULL;
    
    if (STATE & STATE_ANY) {
        printf("** Error, program %s has already been loaded\n", prog.progname);
        return 0;
    }
    ARGC_CHK(argc, 2);
    
    elf_init();
    
    if((eh = elf_open(argv[1])) == NULL) {
		fprintf(stderr, "** unable to open '%s'", argv[1]);
        if (errno) {
            perror(" ");
            return 0;
        }
        printf("\n");
		return 0;
	}
    
    prog.progname = strdup(argv[1]);
    
    if(elf_load_all(eh) < 0) {
        fprintf(stderr, "** unable to load '%s.\n", argv[1]);
        goto quit;
    }
    
    prog.load.entry = eh->entrypoint;

    for(tab = eh->strtab; tab != NULL; tab = tab->next) {
        if(tab->id == eh->shstrndx) break;
    }
    if(tab == NULL) {
        fprintf(stderr, "** section header string table not found.\n");
        goto quit;
    }
    
    for(i = 0; i < eh->shnum; i++) {
        dprintf(1, "%-20s addr: %-12llx off: %-12llx size: %llx\n",
            &tab->data[eh->shdr[i].name],
            eh->shdr[i].addr,
            eh->shdr[i].offset,
            eh->shdr[i].size);
        if (strncmp(".text", &tab->data[eh->shdr[i].name], 5) == 0) {
            prog.load.vaddr = eh->shdr[i].addr;
            prog.load.offset = eh->shdr[i].offset;
            prog.load.size = eh->shdr[i].size;
            goto success;
        }
    }
    dprintf(0, ".text segment not found!\n");
    
quit:
	if(eh) {
		elf_close(eh);
		eh = NULL;
	}
	return -1;
success:
    STATE = STATE_LOAD;
    printf("** program '%s' loaded."
           " entry point 0x%llx, vaddr 0x%llx, offset 0x%llx, size 0x%llx\n"
           , prog.progname, prog.load.entry, prog.load.vaddr
           , prog.load.offset, prog.load.size);
    return 0;
}


static int 
read_mapping (FILE *mapfile, 
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

#define VMMAP_FORMAT_STR "%016llx-%016llx %c%c%c %-10llx %s\n"

int vmmap_show_proc(unsigned int pid){
    FILE *proc;
    char buf[255];
    unsigned long long start, end, off, inode;
    char perm[10];
    char filename[255];
    
    snprintf(buf, 255, "/proc/%d/maps", pid);
    
    dprintf(1, "opening proc file: %s\n", buf);
    
    proc = fopen(buf, "r");
    if (!proc) {
        perror("error opening proc file!\n");
        return -1;
    }
    
    while (read_mapping(proc, &start, &end, perm, &off, buf, &inode, filename)) {
        printf(VMMAP_FORMAT_STR,
               start, end, 
               perm[0],perm[1],perm[2],
               off, filename);
    }
    
    fclose(proc);
    return 0;
}

int vmmap(int argc, char **argv){
    if (!(STATE & STATE_ANY)) {
        printf("** Error, the program must be loaded or started before vmmap\n");
        return 0;
    }
    ARGC_CHK(argc, 1);
    
    if (STATE & STATE_RUNNING) {
        return vmmap_show_proc(prog.pid);
    }
    if (STATE & STATE_LOAD) {
        printf(VMMAP_FORMAT_STR,
               prog.load.entry, prog.load.entry+prog.load.size, 
               'r','-','x',
               prog.load.offset, prog.progname);
        return 0;
    }
    
    return -1;
}