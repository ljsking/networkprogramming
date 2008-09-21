#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <signal.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
 
#define MAXLINE 512 
char *escapechar ="exit"; 
 
int main(int argc, char *argv[]) 
{ 
	int s_d, c_d; 
	int clilen, num; 
	char sendline[MAXLINE], recvline[MAXLINE]; 
	int size; 
	pid_t pid; 
	struct sockaddr_in client_addr, server_addr; 

	if (argc != 2) {
		printf("use : %s port\n", argv[0]); 
		exit(0); 
	} 

	s_d = socket(AF_INET, SOCK_STREAM,0); 
	memset((char *)&server_addr, 0, sizeof(server_addr)); 

	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(atoi(argv[1])); 

	bind(s_d, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	listen(s_d, 1); 
	printf("waiting client\n"); 
	clilen = sizeof(client_addr); 

	if ((c_d = accept(s_d, (struct sockaddr *)&client_addr, &clilen)) < 0) { 
		printf("accept failed\n"); 
		exit(0); 
	} 
	printf("accept client\n"); 
	if ((pid = fork()) > 0) { 
		while(fgets(sendline, MAXLINE, stdin) != NULL) { 
			size = strlen(sendline); 
			if (write(c_d, sendline, strlen(sendline)) != size) { 
				printf("error write\n"); 
				exit(0); 
			} 
			if (strstr(sendline, escapechar) != NULL) { 
				printf("bye~~bye~~~\n"); 
				close(c_d); 
				exit(0); 
			} 
		} 
		exit(0);
	} else if (pid == 0) { 
		while(1) { 
			if ((size = read(c_d, recvline, MAXLINE)) < 0) { 
				printf("error read\n"); 
				close(c_d); 
				exit(0); 
			} 
			recvline[size] = '\0'; 
			if (strstr(recvline, escapechar) != NULL) { 
				printf("client exit\n"); 
				break; 
			} 
			printf("msg : %s\n", recvline); 
		} 
	} 
	close(s_d); 
	close(c_d); 
}