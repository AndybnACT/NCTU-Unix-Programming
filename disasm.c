#include <capstone/capstone.h>
#include <stdio.h>

#include "runcmd.h"
#include "debug.h"
#include "util.h"
#include "break.h"

csh *Handle = NULL;
int capstone_dis(unsigned long long addr);

int dis_asm(int argc, char** argv){
    static unsigned long long addr = -1;

    if (!(STATE & STATE_ANY)) {
        printf("** program must be loaded or run\n");
    }
    if (argc == 1) {
        if (addr == -1) {
            printf("** no addr is given\n");
            return 0;
        }
        return capstone_dis(0);
    }else if (argc == 2) {
        addr = str2num(argv[1]);
        if (addr == -1) {
            printf("** Cannot recognize address\n");
            return -1;
        }
    }else{
        ARGC_CHK(argc, 2);
    }
    dprintf(0, "ready to disasm @ addr = 0x%llx\n", addr);
    
    if (!Handle) {
        Handle = (csh *) malloc(sizeof(csh));
        if (cs_open(CS_ARCH_X86, CS_MODE_64, Handle) != CS_ERR_OK){
            perror("cs_open ");
            return -1;
        }
    }
    
    if (addr >= prog.load.vaddr &&
        addr <= prog.load.vaddr + prog.load.size) { 
        // what about dynamic ELF at runtime
        return capstone_dis(addr);
    }else{
        printf("** addr (%llx) out of bound\n", addr);
        return 0;
    }
    
}

void show_insn(cs_insn *insn, int count){
    for (size_t i = 0; i < count; i++) {
        
        printf("0x%"PRIx64":\t", insn[i].address);
        
        for (size_t j = 0; j < 12; j++) {
            if (j < insn[i].size) {
                printf("%02hhx ", insn[i].bytes[j]);
            }else{
                printf("   ");
            }
        }
        printf("\t%s\t%s\n", insn[i].mnemonic,
					insn[i].op_str);
    }
    return;
}

int _disassemble(struct disasm *dis, unsigned long long addr){
    cs_insn *insn;
    int count;
    unsigned long long start;
    unsigned long long off;
    unsigned long long size;
    
    char *code;
    
    unsigned long long disaddr;
    if (addr) {
        disaddr = addr;
    }else{
        disaddr = dis->cur_va;
    }
    
    start = prog.load.entry;
    off = disaddr - start;
    size = prog.load.size - off;
    code = dis->data + off;
    
    dprintf(1, "disassembling start=%llx, size=%llx\n",
            disaddr, size);
    
    if (size == 0) {
        printf("** no more instruction\n");
        return 0;
    }
    
    count = cs_disasm(*Handle, (uint8_t *) code, size,
                      disaddr, 10, &insn);
    if (count < 0) {
        printf("** error disassembling given code\n");
    }
    dis->cur_va = insn[count-1].address + insn[count-1].size;
    
    show_insn(insn, count);
    
    cs_free(insn, count);                  
    
    return 0;
}


// the function presumes that Handle is opened and STATE is either loaded or running 
int capstone_dis(unsigned long long addr){
    static int checkpoint = 0;
    if (!(STATE & STATE_RUNNING)) { // loaded but not run
        return _disassemble(&prog.asm_file, addr);
    }else { // running
        cs_insn *insn;
        unsigned long long start;
        unsigned long long *rptr;
        int nr;
        struct disasm *dis = &prog.asm_raw;
        if (!checkpoint) {
            dis->size = 1024;
            dis->data = (char*) malloc(dis->size);
            dis->cur_va = prog.asm_file.cur_va + prog.memoff;
            checkpoint = 1;
        }
        start = (addr) ? addr:dis->cur_va;
        
        
        
        // load the entire code area
        nr = dis->size/8;
        break_deactivate_all(prog.b);
        tracee_getmem(start, 
            (unsigned long long*) dis->data,
                &rptr, 
                nr);
        break_activate_all(prog.b);       
        
        if ((char* )rptr - dis->data != dis->size)
            dprintf(0, "reading from child does not complete\n");
        
        
        // disassemble
        nr = cs_disasm(*Handle, (uint8_t*) dis->data, 
                       dis->size, start, 10, &insn);
        if (nr < 0) {
            printf("** error disassembling given code\n");
        }
        show_insn(insn, nr);
        cs_free(insn, nr);
        return 0;
    }
    return -1;
}

int capstone_dis_break(struct breakpoint *b){ // ugly implementation 
    cs_insn *insn;
    int count;
    if (!Handle) {
        Handle = (csh *) malloc(sizeof(csh));
        if (cs_open(CS_ARCH_X86, CS_MODE_64, Handle) != CS_ERR_OK){
            perror("cs_open ");
            return -1;
        }
    }
    
    count = cs_disasm(*Handle, (uint8_t *) &b->data, 8,
                      b->addr, 1, &insn);
    if (count != 1)
        return -1;
    show_insn(insn, 1);
    cs_free(insn, count);
    return 0;
}
