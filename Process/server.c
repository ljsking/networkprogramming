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
	fd_set master;   // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int listener;
	int fdmax;
	
	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);
	
	int clilen, num; 
	char sendline[MAXLINE], recvline[MAXLINE]; 
	int size; 
	pid_t pid; 
	struct sockaddr_in client_addr, server_addr; 

	if (argc != 2) {
		printf("use : %s port\n", argv[0]); 
		exit(0); 
	} 

	listener = socket(AF_INET, SOCK_STREAM,0); 
	memset((char *)&server_addr, 0, sizeof(server_addr)); 

	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(atoi(argv[1])); 

	bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	listen(listener, 1); 
	// add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
	while(1)
	{
		read_fds = master; // copy it
		
	}
}