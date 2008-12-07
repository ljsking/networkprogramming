OBJECTS = server.o util.o simclist.o actions.o
CC = gcc
CFALGS = -g -c -std=c99

TARGET = server

all: server client

server: $(OBJECTS)
	$(CC) -o server -std=c99 $(OBJECTS)
	
client: client.c
	$(CC) -o client -std=c99 client.c
	
clean:
	rm $(OBJECTS) client $(TARGET)
	
server.o: server.c util.h
	$(CC) $(CFALGS) server.c
	
actions.o: actions.c actions.h
	$(CC) $(CFALGS) actions.c
	
util.o: util.c util.h
	$(CC) $(CFALGS) util.c
	
simclist.o: simclist.c simclist.h
	$(CC) $(CFALGS) simclist.c