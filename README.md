## Simple Instruction Level Debugger:
In this homework, we are going to implement a simple instruction-level debugger that allows a user to debug a program interactively at the assembly instruction level. You can implement the debugger by using the ptrace interface. The commands you have to implement are summarized as follows:

## Implemented commands:
All of the commands required by the homework are completed, listing as below: 

 - [x] break {instruction-address}: add a break point
 - [x] cont: continue execution
 - [x] delete {break-point-id}: remove a break point
 - [x] disasm addr: disassemble instructions in a file or a memory region
 - [x] dump addr [length]: dump memory content
 - [x] exit: terminate the debugger
 - [x] get reg: get a single value from a register
 - [x] getregs: show registers
 - [x] help: show this message
 - [x] list: list break points
 - [x] load {path/to/a/program}: load a program
 - [x] run: run the program
 - [x] vmmap: show memory layout
 - [x] set reg val: get a single value to a register
 - [x] si: step into instruction
 - [x] start: start the program and stop at the first instruction
 
## Notes:
 - When using dump command at a break point address right after it is hit, one would notify that memory content at the address is not software interrupt 3 (0xcc in byte stream). The reason is that we have to revert to the original so that restarting from any break point will execute the right code.
 - Setting breakpoint outside of the code region is possible but undefined across multiple runs for a shared-object type ELF executable since we could only recognize the base address of .text section of that object itself.
 - When a breakpoint is disabled by `sdb`, one could re-enable it by setting a breakpoint at the same address. However, if the initial activation of that breakpoint fails, `sdb` will disable it automatically.
 - Except for `SIGSEGV`, `SIGSTOP`, `SIGTSTP`, and `SIGTRAP`, all delivered signals targeting the child process will be sent upon the next restarting-type commands (`si`, `c`, `r`).
