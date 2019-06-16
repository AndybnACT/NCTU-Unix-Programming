#include <stdlib.h>
#include <string.h>
#include "break.h"
#include "debug.h"

#define LIST_FOR_EACH(l) \
    for (l; l; l = l->next)


int Break_ID = 0;

void dbg_show_break(int lvl, struct breakpoint *b) {
    if (lvl > CONFIG_DEBUG_LEVEL) {
        printf("  %d (%s):\t%llx\n"
            ,b->id, b->enable ? "enabled" : "disabled"
            , b->addr);
    }
    // printf("%d, %d\n", lvl, CONFIG_DEBUG_LEVEL);
    dprintf(lvl, "breakpoint id: %d\n", b->id);
    dprintf(lvl, "\t addr: %llx\n", b->addr)
    dprintf(lvl, "\t hit:%d\n", b->hit);
    dprintf(lvl, "\t activated: %d\n", b->activated);
    dprintf(lvl, "\t masked: %d\n", b->masked);
    dprintf(lvl, "\t enabled: %d\n", b->enable);
    dprintf(lvl, "\t data: %016llx\n", b->data);
    dprintf(lvl, "\t next: %p\n", b->next);
}

void dbg_dump_breaks(int lvl, struct breakpoint *head){
    if (!head) {
        dprintf(lvl, "list is empty\n");
    }
#ifdef DEBUG
    lvl = 0;
#endif
    LIST_FOR_EACH(head)
        dbg_show_break(lvl, head);
    return;
}

struct breakpoint* break_init(void){
    struct breakpoint *init;
    init = (struct breakpoint *) malloc(sizeof(struct breakpoint));
    if (!init) {
        perror("malloc ");
        return NULL;
    }
    memset(init, 0, sizeof(struct breakpoint));
    return init;
}

struct breakpoint* break_findby_addr(struct breakpoint *head, unsigned long long addr){
    LIST_FOR_EACH(head){
        if (head->addr == addr) {
            return head;
        }
    }
    dprintf(1, "breakpoint with addr=%llx not found\n", addr);
    return NULL;
}



int break_insert(struct breakpoint **head, struct breakpoint *node){
    if (!*head) {
        dprintf(0, "initializing prog->b\n");
        *head = node;
    }else{
        struct breakpoint *cur = *head;
        if (break_findby_addr(cur, node->addr)) {
            dprintf(0, "breakpoint already exist!\n");
            dbg_show_break(CONFIG_DEBUG_LEVEL-1, cur);
            dprintf(0, "force enabling the breakpoint\n");
            cur->enable = 1;
            return -1;
        }
        LIST_FOR_EACH(cur){
            dbg_show_break(CONFIG_DEBUG_LEVEL-1, cur);
            if (!cur->next)
                break;
        }
        cur->next = node;
    }
    node->id = Break_ID;
    Break_ID++;
    return 0;
}

int break_remove_by_id(struct breakpoint **head, int id){
    struct breakpoint *cur = *head;
    struct breakpoint *freeb;
    struct breakpoint *prev = NULL;
    
    LIST_FOR_EACH(cur){
        if (cur->id == id) {
            freeb = cur;
            if (prev) {
                prev->next = freeb->next;
            }else{
                dprintf(0, "deleting list head\n");
                *head = cur->next;
            }
            
            if (freeb->activated)
                freeb->deactivate(freeb);
            freeb->enable = 0;
            free(freeb);
            return 0;
        }
        prev = cur;
    }
    dprintf(1, "breakpoint id(%d) not found\n", id);
    return -1;
}



int break_hit(struct breakpoint *head, unsigned long long addr){
    struct breakpoint *b;
    if (!head) {
        dprintf(1, "no breakpoint list given\n");
        return -1;
    }
    b = break_findby_addr(head, addr);
    if (!b) 
        return -1;
    if (!b->activated) {
        dprintf(0, "breakpoint is hit but not activated!\n");
        dprintf(0, "possibly due to breaking on single byte instruction\n");
        return -1;
    }else{
        b->hit++;
        b->deactivate(b);
    }
    b->masked = 1;
    b->dis(b);
    return 0;
}

int break_set_offset_all(struct breakpoint *head, long long int off){
    LIST_FOR_EACH(head){
        if (head->activated) {
            dprintf(0, "bug!! function should only be called when breakpoint is deactivated\n");
        }
        head->addr += off;
    }
    return 0;
}

void break_unmask_all(struct breakpoint *head){
    LIST_FOR_EACH(head){
        if (head->masked)
            head->masked = 0;
    }
}

int break_activate_all(struct breakpoint *head){
    int ret;
    LIST_FOR_EACH(head){
        if (head->enable && !head->activated && !head->masked) {
            ret = head->activate(head);
            if (ret){
                printf("** cannot insert breakpoint %d @ 0x%llx currently, disabling it\n",
                    head->id, head->addr);
                printf("** (Future: pending breakpoint for lib load)\n");
                head->enable = 0;
            }
        }
    }
    return 0;
}


int break_deactivate_all(struct breakpoint *head){
    int ret;
    LIST_FOR_EACH(head){
        if (head->enable && head->activated && !head->masked) {
            ret = head->deactivate(head);
            if (ret){
                dprintf(0, "bug! cannot deactivate breakpoint\n");
                dbg_show_break(CONFIG_DEBUG_LEVEL-1, head);
            }
        }
    }
    return 0;
}

// only be called when process terminated
int break_clear_activate_all(struct breakpoint *head){
    // int ret;
    LIST_FOR_EACH(head){
        if (head->enable && head->activated) {
            head->activated = 0;
            // we could call head->deactivate() here and let it fail silencely
            // when tracee exit
        }
    }
    return 0;
}

