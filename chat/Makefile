CC = gcc

all:
	$(CC) src/c_chat.c src/client.c -o client
	$(CC) src/c_chat.c src/server.c -o server

clean:
	rm -f client server



.PHONY: all clean