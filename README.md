<!-- ## Usage: 
Different homeworks are catagorized as branches. To navigate through HWs, please simply checkout the branch you wish. -->

## Homework 3 for Unix Programming @ NCTU, 2019
  - In this homework, we have to extend the mini C library introduced in the class to support signal relevant system calls. We have to implement the following C library functions in Assembly and C using the syntax supported by yasm x86_64 assembler.
    1. setjmp: prepare for long jump by saving the current CPU state. In addition, preserve the signal mask of the current process.
    2. longjmp: perform the long jump by restoring a saved CPU state. In addition, restore the preserved signal mask.
    3. signal and sigaction: setup the handler of a signal.
    4. sigprocmask: can be used to block/unblock signals, and get/set the current signal mask.
    5. sigpending: check if there is any pending signal.
    6. alarm: setup a timer for the current process.
    7. functions to handle sigset_t data type: sigemptyset, sigfillset, sigaddset, sigdelset, and sigismember.

 
## Implemented Features
- [x] [12%] alarm
- [x] [12%] sigprocmask
- [x] [12%] sigpending
- [x] [12%] functions to handle sigset_t data type
- [x] [20%] setjmp and longjmp
- [x] [20%] signal and sigaction
- [ ] [22%] extra (unpublished) test cases
## Building the Library
  - To build the library, simply `make` at this directory. Test cases are placed in `./testcase` and can be built by `make test`.