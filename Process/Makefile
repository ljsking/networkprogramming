all: client server server_simple

client: client.c
	gcc -o client client.c
	
server: server.c
	gcc -o server server.c

server_simple: server_simple.c
	gcc -o server_simple server_simple.c

clean:
	rm -f client
	rm -f server
	rm -f server_simple
