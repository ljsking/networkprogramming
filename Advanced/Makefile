all: server_tpool client

clean:
	rm server
	rm client

server_tpool: server_tpool.cpp
	${CPP} -o server server_tpool.cpp -lpthread
	
client: client.c
	${CC} -o client client.c

CC = $(PREFIX)gcc
CPP = $(PREFIX)g++
