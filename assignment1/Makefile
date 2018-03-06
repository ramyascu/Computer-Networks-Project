CC = gcc
DEPS = common.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

all: server client

server: server.o
	$(CC) -o $@ $^

client: client.o 
	$(CC) -o $@ $^

clean:
	rm -f *.o server client
