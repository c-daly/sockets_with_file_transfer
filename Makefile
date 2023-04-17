all: server client

server: server.c utils.c
	gcc server.c utils.c -o server -lpthread

client: client.c utils.c
	gcc client.c utils.c -o client

clean:
	rm client; rm server;

debug_server:
	gcc server.c utils.c -g -o server -lpthread
