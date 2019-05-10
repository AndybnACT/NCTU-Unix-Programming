CC      = gcc
CFLAGS  = -g -Wall -fno-stack-protector -nostdlib
LD      = ld
LDFLAGS = -m elf_x86_64 --dynamic-linker /lib64/ld-linux-x86-64.so.2

TEST	= alarm1 alarm2 alarm3 jmp1

all: $(TEST)

clean:
	rm $(TEST)
	rm alarm1.o alarm2.o alarm3.o jmp1.o 

%.o: %.c
	$(CC) -c $(CFLAGS) -I. -I.. -DUSEMINI $< -o $@
alarm1: alarm1.o
	ld $(LDFLAGS) -o $@ $< ../start.o -L. -L.. -lmini
	rm $<
alarm2: alarm2.o
	ld $(LDFLAGS) -o $@ $< ../start.o -L. -L.. -lmini
	rm $<
alarm3: alarm3.o
	ld $(LDFLAGS) -o $@ $< ../start.o -L. -L.. -lmini
	rm $<
jmp1: jmp1.o
	ld $(LDFLAGS) -o $@ $< ../start.o -L. -L.. -lmini