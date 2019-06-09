TARGET = sdb
SOURCE = main.c 
OBJ    = main.o runcmd.o load.o elftool.o sdb-core.o

CC = gcc
CFLAGS = -Wall -g 
RM = rm

all: $(TARGET)

clean:
	$(RM) $(OBJ)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -lelf $(OBJ) -o $(TARGET)

load.o: load.c runcmd.h elftool.o
	$(CC) $(CFLAGS) -c load.c -o load.o

elfdemo: elfdemo.c elftool.o
	$(CC) $(CFLAGS) -c elfdemo.c -o elfdemo.o
	$(CC) $(CFLAGS) -lelf elfdemo.o elftool.o -o elfdemo

elftool.o:	elftool.c elftool.h
	$(CC) $(CFLAGS) -c elftool.c -o elftool.o

%.o: %.c %.h debug.h runcmd.h
	$(CC)  $(CFLAGS) -c $< -o $@