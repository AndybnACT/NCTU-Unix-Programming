#include <sys/user.h>

struct REGS{
    char* name;
    unsigned long long (*get)(struct user_regs_struct *);
    void (*set)(struct user_regs_struct *, unsigned long long);
};



inline static unsigned long long  get_r15(struct user_regs_struct *r){ return r->r15;      }
inline static unsigned long long  get_r14(struct user_regs_struct *r){ return r->r14;      }
inline static unsigned long long  get_r13(struct user_regs_struct *r){ return r->r13;      }
inline static unsigned long long  get_r12(struct user_regs_struct *r){ return r->r12;      }
inline static unsigned long long  get_rbp(struct user_regs_struct *r){ return r->rbp;      }
inline static unsigned long long  get_rbx(struct user_regs_struct *r){ return r->rbx;      }
inline static unsigned long long  get_r11(struct user_regs_struct *r){ return r->r11;      }
inline static unsigned long long  get_r10(struct user_regs_struct *r){ return r->r10;      }
inline static unsigned long long  get_r9(struct user_regs_struct *r){ return r->r9;       }
inline static unsigned long long  get_r8(struct user_regs_struct *r){ return r->r8;       }
inline static unsigned long long  get_rax(struct user_regs_struct *r){ return r->rax;      }
inline static unsigned long long  get_rcx(struct user_regs_struct *r){ return r->rcx;      }
inline static unsigned long long  get_rdx(struct user_regs_struct *r){ return r->rdx;      }
inline static unsigned long long  get_rsi(struct user_regs_struct *r){ return r->rsi;      }
inline static unsigned long long  get_rdi(struct user_regs_struct *r){ return r->rdi;      }
inline static unsigned long long  get_orig_rax(struct user_regs_struct *r){ return r->orig_rax; }
inline static unsigned long long  get_rip(struct user_regs_struct *r){ return r->rip;      }
inline static unsigned long long  get_cs(struct user_regs_struct *r){ return r->cs;       }
inline static unsigned long long  get_eflags(struct user_regs_struct *r){ return r->eflags;   }
inline static unsigned long long  get_rsp(struct user_regs_struct *r){ return r->rsp;      }
inline static unsigned long long  get_ss(struct user_regs_struct *r){ return r->ss;       }
inline static unsigned long long  get_fs_base(struct user_regs_struct *r){ return r->fs_base;  }
inline static unsigned long long  get_gs_base(struct user_regs_struct *r){ return r->gs_base;  }
inline static unsigned long long  get_ds(struct user_regs_struct *r){ return r->ds;       }
inline static unsigned long long  get_es(struct user_regs_struct *r){ return r->es;       }
inline static unsigned long long  get_fs(struct user_regs_struct *r){ return r->fs;       }
inline static unsigned long long  get_gs(struct user_regs_struct *r){ return r->gs;       }


inline static void set_r15(struct user_regs_struct *r, unsigned long long val){ r->r15 = val; return;      }
inline static void set_r14(struct user_regs_struct *r, unsigned long long val){ r->r14 = val; return;      }
inline static void set_r13(struct user_regs_struct *r, unsigned long long val){ r->r13 = val; return;      }
inline static void set_r12(struct user_regs_struct *r, unsigned long long val){ r->r12 = val; return;      }
inline static void set_rbp(struct user_regs_struct *r, unsigned long long val){ r->rbp = val; return;      }
inline static void set_rbx(struct user_regs_struct *r, unsigned long long val){ r->rbx = val; return;      }
inline static void set_r11(struct user_regs_struct *r, unsigned long long val){ r->r11 = val; return;      }
inline static void set_r10(struct user_regs_struct *r, unsigned long long val){ r->r10 = val; return;      }
inline static void set_r9(struct user_regs_struct *r, unsigned long long val){ r->r9 = val; return;       }
inline static void set_r8(struct user_regs_struct *r, unsigned long long val){ r->r8 = val; return;       }
inline static void set_rax(struct user_regs_struct *r, unsigned long long val){ r->rax = val; return;      }
inline static void set_rcx(struct user_regs_struct *r, unsigned long long val){ r->rcx = val; return;      }
inline static void set_rdx(struct user_regs_struct *r, unsigned long long val){ r->rdx = val; return;      }
inline static void set_rsi(struct user_regs_struct *r, unsigned long long val){ r->rsi = val; return;      }
inline static void set_rdi(struct user_regs_struct *r, unsigned long long val){ r->rdi = val; return;      }
inline static void set_orig_rax(struct user_regs_struct *r, unsigned long long val){ r->orig_rax = val; return; }
inline static void set_rip(struct user_regs_struct *r, unsigned long long val){ r->rip = val; return;      }
inline static void set_cs(struct user_regs_struct *r, unsigned long long val){ r->cs = val; return;       }
inline static void set_eflags(struct user_regs_struct *r, unsigned long long val){ r->eflags = val; return;   }
inline static void set_rsp(struct user_regs_struct *r, unsigned long long val){ r->rsp = val; return;      }
inline static void set_ss(struct user_regs_struct *r, unsigned long long val){ r->ss = val; return;       }
inline static void set_fs_base(struct user_regs_struct *r, unsigned long long val){ r->fs_base = val; return;  }
inline static void set_gs_base(struct user_regs_struct *r, unsigned long long val){ r->gs_base = val; return;  }
inline static void set_ds(struct user_regs_struct *r, unsigned long long val){ r->ds = val; return;       }
inline static void set_es(struct user_regs_struct *r, unsigned long long val){ r->es = val; return;       }
inline static void set_fs(struct user_regs_struct *r, unsigned long long val){ r->fs = val; return;       }
inline static void set_gs(struct user_regs_struct *r, unsigned long long val){ r->gs = val; return;       }


struct REGS REGS[] ={
    {"r15",     get_r15,          set_r15          }, 
    {"r14",     get_r14,          set_r14          }, 
    {"r13",     get_r13,          set_r13          }, 
    {"r12",     get_r12,          set_r12          }, 
    {"rbp",     get_rbp,          set_rbp          }, 
    {"rbx",     get_rbx,          set_rbx          }, 
    {"r11",     get_r11,          set_r11          }, 
    {"r10",     get_r10,          set_r10          }, 
    {"r9",      get_r9,           set_r9           }, 
    {"r8",      get_r8,           set_r8           }, 
    {"rax",     get_rax,          set_rax          }, 
    {"rcx",     get_rcx,          set_rcx          }, 
    {"rdx",     get_rdx,          set_rdx          }, 
    {"rsi",     get_rsi,          set_rsi          }, 
    {"rdi",     get_rdi,          set_rdi          }, 
    {"orig_rax", get_orig_rax,    set_orig_rax     }, 
    {"rip",     get_rip,          set_rip          }, 
    {"cs",      get_cs,           set_cs           }, 
    {"eflags",  get_eflags,       set_eflags       }, 
    {"rsp",     get_rsp,          set_rsp          }, 
    {"ss",      get_ss,           set_ss           }, 
    {"fs_base", get_fs_base,      set_fs_base      }, 
    {"gs_base", get_gs_base,      set_gs_base      }, 
    {"ds",      get_ds,           set_ds           }, 
    {"es",      get_es,           set_es           }, 
    {"fs",      get_fs,           set_fs           }, 
    {"gs",      get_gs,           set_gs           }      
};
const int NREGS = (sizeof(REGS)/sizeof(struct REGS));

