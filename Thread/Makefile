all: thread thread_self

thread: thread.c
	gcc -o thread thread.c -lpthread
	
thread_self: thread_self.c
	gcc -o thread_self thread_self.c -lpthread -D_REENTRANT
	
clean:
	rm -f thread
	rm -f thread_self
