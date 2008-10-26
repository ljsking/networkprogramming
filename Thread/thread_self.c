#include <pthread.h> 
#include <stdio.h> 

void *function(void *arg); 

int main(void) 
{ 
	void *returnvalue;
	pthread_t p_thread; 
	pthread_create(&p_thread, NULL, function, (void *)NULL);
	printf("ret : %d\n", p_thread); 
	pthread_join(p_thread, &returnvalue); 
	return 1; 
}
 
void *function(void *arg) 
{ 
	pthread_t p_pthread; 
	p_pthread = pthread_self(); 
	printf("function ret : %d\n", p_pthread); 
} 