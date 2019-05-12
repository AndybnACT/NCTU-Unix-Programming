CC      = gcc
CFLAGS  = -O0 -g -Wall -fno-stack-protector -nostdlib
LD      = ld
LDFLAGS = -m elf_x86_64 --dynamic-linker /lib64/ld-linux-x86-64.so.2

TEST	= alarm1 alarm2 alarm3 jmp1 jmp2 sig1

all: $(TEST)

clean:
	rm $(TEST)


%: %.c
	$(CC) -c $(CFLAGS) -I. -I.. -DUSEMINI $< -o tmp.o
	ld $(LDFLAGS) -o $@ tmp.o ../start.o -L. -L.. -lmini
	rm tmp.o
