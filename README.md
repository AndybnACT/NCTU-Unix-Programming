## Homework 1 for Unix Programming @ NCTU, 2019
 - The homework requires us to implement a `netstat -nap` from scratch.
 
## Homework requirement:
 - Command line: `$ ./hw1 [-t|--tcp] [-u|--udp] [filter-string]`
 - [10%] List TCP and UDP connetions (IPv4).
 - [10%] List TCP and UDP connetions (IPv6).
 - [30%] Show corresponding command lines and arguments for each identified connection.
 - [10%] Implement -u and --udp option using getopt_long(3).
 - [10%] Implement -t and --tcp option using getopt_long(3).
 - [10%] Translate network address into user-friendly formats, e.g., from 0100007F to 127.0.0.1, and from FE01A8C0 to 192.168.1.254.
 - [10%] Implement basic filter string feature.
 - [10%] Use Makefile to manage the building process of your program.
 - [10%] If your filter string supports regular expression, see regex(3).

## Current Status
All done.

## Building the Program
Just type `make` at current directory, then the program will be built accordingly. To see debug outputs, please refer to `netstat.h`, uncomment `CONFIG_DEBUG` and set a preferred debug level.
