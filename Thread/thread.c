#include <pthread.h> 
#include <stdio.h> 
#include <sys/types.h> 
 
#define thread_num 3 
 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
 
void *thread_function(void *arg); 
 
int main(void) 
{ 
	int ret; 
	pthread_t p_thread[thread_num]; 
	void *return_value; 
	int i, j; 
	pid_t pid; 

	pid = getpid(); 
	for (i =0; i < thread_num; i++) { 
		ret = pthread_create(&p_thread[i], NULL, thread_function, (void *)NULL); 
		if (ret < 0) { 
			printf("Thread creation failed\n"); 
			exit(0); 
		}  
		printf("process_id[%d] - thread_num[%d] : ret[%d]\n", pid, i, p_thread[i]); 
		sleep(1); 
	}  
	printf("waiting for thread termination\n"); 
	for (j = 1; j <thread_num; j++) { 
		ret = pthread_join(p_thread[j], &return_value); 
		if (ret < 0) { 
			printf("pthread_join failed\n"); 
			exit(0); 
		} 
		printf("process_id[%d] - thread_num[%d] : thread_join[%d] is end\n", 
		pid, j, p_thread[j]); 
	} 
	printf("end\n"); 
	pthread_mutex_destroy(&mutex); 
} 

void *thread_function(void *arg) 
{ 
	int i; 
	pid_t p_pid; 
	pthread_t p_pthread; 
	p_pid = getpid(); 
	p_pthread = pthread_self(); 

	pthread_mutex_lock(&mutex); 
	for (i=0; i<5; i++) 
	{ 
		sleep(1); 
		printf("process[%d] - loop_num[%d] : thread[%d]\n", p_pid, i, p_pthread); 
	} 
	pthread_mutex_unlock(&mutex); 
	return(arg); 
} 
