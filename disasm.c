#include <capstone/capstone.h>
#include <stdlib.h>
#include <stdio.h>

#include "runcmd.h"
#include "debug.h"
#include "util.h"
#include "break.h"

csh *Handle = NULL;
int capstone_dis(struct disasm *dmp);

int dis_asm(int argc, char** argv){
    unsigned long long addr = -1;
    struct disasm *dmp;
    // initialize capstone engine
    if (!Handle) {
        Handle = (csh *) malloc(sizeof(csh));
        if (cs_open(CS_ARCH_X86, CS_MODE_64, Handle) != CS_ERR_OK){
            perror("cs_open ");
            return -1;
        }
    }

    // check state & argc
    if (!(STATE & STATE_ANY)) {
        printf("** program must be loaded or run\n");
        return 0;
    }
    
    // prepare struct disasm
    if (STATE & STATE_RUNNING) {
        dmp = &prog.asm_raw;
    }else{
        dmp = &prog.asm_file;
    }
    
    if (argc == 1) {
        if (!dmp->dumpped) {
            printf("** no addr is given\n");
            return 0;
        }
        addr = dmp->cur_va;
    }else if (argc == 2) {
        addr = str2num(argv[1]);
        if (addr == -1) {
            printf("** Cannot recognize address\n");
            return 0;
        }
        dmp->cur_va = addr;
    }else{
        ARGC_CHK(argc, 2);
    }
    
    // call disassembler
    if (STATE & STATE_RUNNING)
        return capstone_dis(dmp);

    if (addr >= prog.load.vaddr &&
        addr <= prog.load.vaddr + prog.load.size) { 
        // what about dynamic ELF at runtime
        return capstone_dis(dmp);
    }else{
        printf("** addr (%llx) out of bound\n", addr);
        return 0;
    }
    
}

void show_insn(cs_insn *insn, int count){
    for (size_t i = 0; i < count; i++) {
        
        printf("\t0x%"PRIx64":\t", insn[i].address);
        
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

int _disassemble(struct disasm *dis){
    cs_insn *insn;
    int count;
    unsigned long long off;
    unsigned long long size;
    
    char *code;
    
    unsigned long long disaddr;


    disaddr = dis->cur_va;

    off = dis->dat_off;
    size = dis->size - off;
    code = dis->data + off;
    
    if (size == 0) {
        printf("** no more instruction\n");
        return 0;
    }
    
    dprintf(1, "disassembling start=%llx, size=%llx\n",
            disaddr, size);
    count = cs_disasm(*Handle, (uint8_t *) code, size,
                      disaddr, 10, &insn);
    dprintf(1, "complete disassembling, count %d\n", count);
    if (count < 0) {
        printf("** error disassembling given code\n");
    }
    else if (count) {
        dis->cur_va = insn[count-1].address + insn[count-1].size;
        show_insn(insn, count);
        dis->dumpped++;
        cs_free(insn, count);  
    }
    return 0;
}


// the function presumes that Handle is opened and STATE is either loaded or running 
int capstone_dis(struct disasm *dmp){
    if (!(STATE & STATE_RUNNING)) { // loaded but not run
        dmp->dat_off = dmp->cur_va - prog.load.vaddr;
        return _disassemble(dmp);
    }else { // running
        unsigned long long start = dmp->cur_va;
        unsigned long long *rptr;
        int nr;
        if (!dmp->data) {
            dmp->size = 1024;
            dmp->data = (char*) malloc(dmp->size);
        }
        dmp->dat_off = 0;
    
        // load the entire code area
        nr = dmp->size/8;
        break_deactivate_all(prog.b);
        nr = tracee_getmem(start, 
            (unsigned long long*) dmp->data,
                &rptr, 
                nr);
        break_activate_all(prog.b);       
    
        if ((char* )rptr - dmp->data != dmp->size)
            dprintf(0, "reading from child does not complete\n");
        dmp->size = (char* )rptr - dmp->data;
        
        // disassemble
        return  _disassemble(dmp);
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
    
    printf("** breakpoint @ ");
    count = cs_disasm(*Handle, (uint8_t *) &b->data, 8,
                      b->addr, 1, &insn);
    if (count != 1)
        return -1;
    show_insn(insn, 1);
    cs_free(insn, count);
    return 0;
}
