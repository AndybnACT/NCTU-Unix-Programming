CC		= gcc
ASM64	= yasm -f elf64 -DYASM -D__x86_64__ -DPIC
CFLAGS  = -g -Wall -fno-stack-protector -nostdlib

LIB		= libmini.so
OBJS	= libmini64.o libmini.o start.o


all: $(LIB) $(OBJS)

test:
	make -C testcase


clean:
	rm $(LIB) 
	rm $(OBJS)

%.o: %.asm
	$(ASM64) $< -o $@
%.o: %.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

$(LIB): libmini64.o libmini.o 
	ld -shared -o $@ libmini64.o libmini.o
	