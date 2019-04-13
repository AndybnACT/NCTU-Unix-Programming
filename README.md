<!-- ## Usage: 
Different homeworks are categorized as branches. To navigate through HWs, please simply checkout the branch you wish. -->
## Homework 2 for Unix Programming @ NCTU, 2019
  - In this homework, we have to monitor file and directory activities of dynamically linked programs. 

## Minimum functions required to be monitored:
```
closedir, opendir, readdir, creat, open, read, write, dup,
dup2, close, lstat, stat, pwrite, fopen, fclose, fread,
fwrite, fgetc, fgets, fscanf, fprintf, chdir, chown, chmod,
remove, rename, link, unlink, readlink, symlink, mkdir, rmdir
```
-  Functions that currently implemented:
    - All functions listed above are implemented, plus the followings:
    ```
    fflush, fputs_unlocked, fwrite_unlocked
    ```
    - Our implementation allows users to easily add new functions and expand the variety of logging types. Detail steps to add a new monitored function are stated at section 'Development Notes'.

## Grading Policy
- [x] [10%] A monitored executable can work as usual. Your program cannot break the functions of a monitored executable.
- [x] [20%] Monitor functions listed in minimum requirements.
- [x] [20%] Provide basic list for function call parameters and return values.
- [x] [20%] Provide comprehensive list for function call parameters and return values.
- [x] [20%] Output can be configured using MONITOR_OUTPUT environmental variable.
- [x] [10%] Use Makefile to manage the building process of your program. We will not grade your program if we cannot use make command to build your program.

---
## Development Notes:
- To add a new injected function, please follow the instructions below:
     1. `INJECT_DEFINE()` the function, type of arguments and return value should be prefix with TYPE or TYPEP if it is a pointer.
     2. `typedef` the type at `fsmon.c` if there is a new type.
     3. if a new type is used, please define and implement the corresponding logging routine, whose name are the type itself and prefixed with `log_`, at `common.h` and `logger.c`.
     * Do not include header file of injected function in fsmon.c or you will get name conflict.
 - Current status:
    * All Done!
 - Limitation:
    * Due to the hard-coded macros, current implementation only allows we to inject functions with number of arguments up to 9. To increase the number of supported arguments, please set `MAX_ARGS` at `gen_va_macro.sh` to an appropriate value.
 - To expand the implementation onto multi-thread applications, all global variables should be placed at Thread-Local-Storage. Otherwise, race conditions may break the program.
