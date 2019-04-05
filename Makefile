CC = gcc
CFLAGS =
RM = rm


all: hw1
clean:
	$(RM) netstat netstat.o parse_net.o search.o select_net.o result.o
	
	
hw1: netstat.o parse_net.o search.o select_net.o result.o
	$(CC) $(CFLAGS) -o hw1 netstat.o parse_net.o search.o select_net.o result.o
	
	
netstat.o: netstat.c netstat.h parse_net.h select_net.h
	$(CC) $(CFLAGS) -c netstat.c

parse_net.o: parse_net.c netstat.h parse_net.h
	$(CC) $(CFLAGS) -c parse_net.c  

search.o: search.c search.h netstat.h parse_net.h
	$(CC) $(CFLAGS) -c search.c
	
select_net.o: select_net.c select_net.h netstat.h parse_net.h 
	$(CC) $(CFLAGS) -c select_net.c
	
result.o: result.c result.h netstat.h parse_net.h
	$(CC) $(CFLAGS) -c result.c