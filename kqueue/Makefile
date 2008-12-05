OBJECTS = server.o util.o simclist.o
CC = gcc
CFALGS = -g -c -std=c99

TARGET = server

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) -std=c99 $(OBJECTS)
	
clean:
	rm $(OBJECTS) $(TARGET)
	
server.o: server.c util.h
	$(CC) $(CFALGS) server.c
	
util.o: util.c util.h
	$(CC) $(CFALGS) util.c
	
simclist.o: simclist.c simclist.h
	$(CC) $(CFALGS) simclist.c