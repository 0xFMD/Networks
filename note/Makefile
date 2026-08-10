CC = gcc

all:
	$(CC)  src/client.c -o client -l asound
	$(CC)  src/server.c -o server

clean:
	rm -f client server



.PHONY: all clean