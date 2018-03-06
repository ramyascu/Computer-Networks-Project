CC = gcc
DEPS = common.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

server: server.o
	$(CC) -o $@ $^

client: client.o 
	$(CC) -o $@ $^
