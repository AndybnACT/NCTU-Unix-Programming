#ifndef BREAK_H
#define  BREAK_H


struct breakpoint{
    int id;
    int hit;
    int enable;
    int masked; // mask the bp when hit
    int activated;
    unsigned long long addr;
    unsigned long long data;
    struct breakpoint *next;
    int (*activate)(struct breakpoint*);
    int (*deactivate)(struct breakpoint*);
    int (*dis)(struct breakpoint*); // disassemble
};

void dbg_show_break(int lvl, struct breakpoint *b);
void dbg_dump_breaks(int lvl, struct breakpoint *head);
struct breakpoint* break_init(void);
int break_insert(struct breakpoint **head, struct breakpoint *node);
int break_remove_by_id(struct breakpoint **head, int id);
// struct breakpoint* break_findby_addr(struct breakpoint *head, unsigned long long addr);
int break_hit(struct breakpoint *head, unsigned long long addr);
int break_set_offset_all(struct breakpoint *head, long long int off);
int break_activate_all(struct breakpoint *head);
int break_deactivate_all(struct breakpoint *head);
void break_unmask_all(struct breakpoint *head);
int break_clear_activate_all(struct breakpoint *head);
int capstone_dis_break(struct breakpoint *b);
#endif
